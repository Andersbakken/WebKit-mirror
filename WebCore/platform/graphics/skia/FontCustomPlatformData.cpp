/*
 * Copyright (C) 2007 Apple Computer, Inc.
 * Copyright (c) 2007, 2008, 2009, Google Inc. All rights reserved.
 * Copyright (C) 2010 Company 100, Inc.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FontCustomPlatformData.h"

#if OS(WINDOWS)
#include "Base64.h"
#include "ChromiumBridge.h"
#include "OpenTypeUtilities.h"
#elif OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
#include "SkStream.h"
#endif

#include "FontPlatformData.h"
#include "NotImplemented.h"
#include "OpenTypeSanitizer.h"
#include "SharedBuffer.h"

#if OS(WINDOWS)
#include <objbase.h>
#elif OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
#include <cstring>
#endif

namespace WebCore {

FontCustomPlatformData::~FontCustomPlatformData()
{
#if OS(WINDOWS)
    if (m_fontReference)
        RemoveFontMemResourceEx(m_fontReference);
#elif OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
    if (m_fontReference)
        m_fontReference->unref();
#endif
}

FontPlatformData FontCustomPlatformData::fontPlatformData(int size, bool bold, bool italic, FontOrientation orientation, FontRenderingMode mode)
{
#if OS(WINDOWS)
    ASSERT(m_fontReference);

    LOGFONT logFont;
    // m_name comes from createUniqueFontName, which, in turn, gets
    // it from base64-encoded uuid (128-bit). So, m_name
    // can never be longer than LF_FACESIZE (32).
    if (m_name.length() + 1 >= LF_FACESIZE) {
        ASSERT_NOT_REACHED();
        return FontPlatformData();
    }
    memcpy(logFont.lfFaceName, m_name.charactersWithNullTermination(),
           sizeof(logFont.lfFaceName[0]) * (1 + m_name.length()));

    // FIXME: almost identical to FillLogFont in FontCacheWin.cpp.
    // Need to refactor. 
    logFont.lfHeight = -size;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfUnderline = false;
    logFont.lfStrikeOut = false;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    logFont.lfQuality = ChromiumBridge::layoutTestMode() ?
                        NONANTIALIASED_QUALITY :
                        DEFAULT_QUALITY; // Honor user's desktop settings.
    logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    logFont.lfItalic = italic;
    logFont.lfWeight = bold ? 700 : 400;

    HFONT hfont = CreateFontIndirect(&logFont);
    return FontPlatformData(hfont, size);
#elif OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
    ASSERT(m_fontReference);
    return FontPlatformData(m_fontReference, "", size, bold && !m_fontReference->isBold(), italic && !m_fontReference->isItalic(), orientation);
#else
    notImplemented();
    return FontPlatformData();
#endif
}

#if OS(WINDOWS)
// Creates a unique and unpredictable font name, in order to avoid collisions and to
// not allow access from CSS.
static String createUniqueFontName()
{
    Vector<char> fontUuid(sizeof(GUID));
    CoCreateGuid(reinterpret_cast<GUID*>(fontUuid.data()));

    Vector<char> fontNameVector;
    base64Encode(fontUuid, fontNameVector);
    ASSERT(fontNameVector.size() < LF_FACESIZE);
    return String(fontNameVector.data(), fontNameVector.size());
}
#endif

#if OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
class RemoteFontStream : public SkStream {
public:
    explicit RemoteFontStream(PassRefPtr<SharedBuffer> buffer)
        : m_buffer(buffer)
        , m_offset(0)
    {
    }

    virtual ~RemoteFontStream()
    {
    }

    virtual bool rewind()
    {
        m_offset = 0;
        return true;
    }

    virtual size_t read(void* buffer, size_t size)
    {
        if (!buffer && !size) {
            // This is request for the length of the stream.
            return m_buffer->size();
        }
        if (!buffer) {
            // This is a request to skip bytes. This operation is not supported.
            return 0;
        }
        // This is a request to read bytes.
        if (!m_buffer->data() || !m_buffer->size())
            return 0;
        size_t left = m_buffer->size() - m_offset;
        size_t toRead = (left > size) ? size : left;
        std::memcpy(buffer, m_buffer->data() + m_offset, toRead);
        m_offset += toRead;
        return toRead;
    }

private:
    RefPtr<SharedBuffer> m_buffer;
    size_t m_offset;
};
#endif

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer)
{
    ASSERT_ARG(buffer, buffer);

#if ENABLE(OPENTYPE_SANITIZER)
    OpenTypeSanitizer sanitizer(buffer);
    RefPtr<SharedBuffer> transcodeBuffer = sanitizer.sanitize();
    if (!transcodeBuffer)
        return 0; // validation failed.
    buffer = transcodeBuffer.get();
#endif

#if OS(WINDOWS)
    // Introduce the font to GDI. AddFontMemResourceEx should be used with care, because it will pollute the process's
    // font namespace (Windows has no API for creating an HFONT from data without exposing the font to the
    // entire process first).
    String fontName = createUniqueFontName();
    HANDLE fontReference = renameAndActivateFont(buffer, fontName);
    if (!fontReference)
        return 0;
    return new FontCustomPlatformData(fontReference, fontName);
#elif OS(LINUX) || OS(FREEBSD) || PLATFORM(BREWMP)
    RemoteFontStream* stream = new RemoteFontStream(buffer);
    SkTypeface* typeface = SkTypeface::CreateFromStream(stream);
    if (!typeface)
        return 0;
    return new FontCustomPlatformData(typeface);
#else
    notImplemented();
    return 0;
#endif
}

bool FontCustomPlatformData::supportsFormat(const String& format)
{
    return equalIgnoringCase(format, "truetype") || equalIgnoringCase(format, "opentype")
#if ENABLE(OPENTYPE_SANITIZER)
        || equalIgnoringCase(format, "woff")
#endif
    ;
}

}
