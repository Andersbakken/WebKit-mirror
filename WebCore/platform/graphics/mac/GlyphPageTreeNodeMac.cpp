/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "GlyphPageTreeNode.h"
#include "Font.h"

#include "SimpleFontData.h"
#include "WebCoreSystemInterface.h"
#include <ApplicationServices/ApplicationServices.h>

namespace WebCore {

#ifndef BUILDING_ON_TIGER
static bool shouldUseCoreText(UChar* buffer, unsigned bufferLength, const SimpleFontData* fontData)
{
    if (fontData->orientation() == Vertical && !fontData->isBrokenIdeographFont()) {
        // Ideographs don't have a vertical variant.
        for (unsigned i = 0; i < bufferLength; ++i) {
            if (!Font::isCJKIdeograph(buffer[i]))
                return true;
        }
    }

    return false;
}
#endif

bool GlyphPage::fill(unsigned offset, unsigned length, UChar* buffer, unsigned bufferLength, const SimpleFontData* fontData)
{
    bool haveGlyphs = false;

#ifndef BUILDING_ON_TIGER
    if (!shouldUseCoreText(buffer, bufferLength, fontData)) {
        Vector<CGGlyph, 512> glyphs(bufferLength);
        wkGetGlyphsForCharacters(fontData->platformData().cgFont(), buffer, glyphs.data(), bufferLength);
        for (unsigned i = 0; i < length; ++i) {
            if (!glyphs[i])
                setGlyphDataForIndex(offset + i, 0, 0);
            else {
                setGlyphDataForIndex(offset + i, glyphs[i], fontData);
                haveGlyphs = true;
            }
        }
    } else {
        // We ask CoreText for possible vertical variant glyphs
        RetainPtr<CFStringRef> string(AdoptCF, CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault, buffer, bufferLength, kCFAllocatorNull));
        RetainPtr<CFAttributedStringRef> attributedString(AdoptCF, CFAttributedStringCreate(kCFAllocatorDefault, string.get(), fontData->getCFStringAttributes(0)));
        RetainPtr<CTLineRef> line(AdoptCF, CTLineCreateWithAttributedString(attributedString.get()));

        CFArrayRef runArray = CTLineGetGlyphRuns(line.get());
        CFIndex runCount = CFArrayGetCount(runArray);

        // Initialize glyph entries
        for (unsigned index = 0; index < length; ++index)
            setGlyphDataForIndex(offset + index, 0, 0);

        Vector<CGGlyph, 512> glyphVector;
        Vector<CFIndex, 512> indexVector;
        bool done = false;
        for (CFIndex r = 0; r < runCount && !done ; ++r) {
            // CTLine could map characters over multiple fonts using its own font fallback list.
            // We need to pick runs that use the exact font we need, i.e., fontData->platformData().ctFont().
            CTRunRef ctRun = static_cast<CTRunRef>(CFArrayGetValueAtIndex(runArray, r));
            ASSERT(CFGetTypeID(ctRun) == CTRunGetTypeID());

            CFDictionaryRef attributes = CTRunGetAttributes(ctRun);
            CTFontRef runFont = static_cast<CTFontRef>(CFDictionaryGetValue(attributes, kCTFontAttributeName));
            RetainPtr<CGFontRef> runCGFont(AdoptCF, CTFontCopyGraphicsFont(runFont, 0));
            // Use CGFont here as CFEqual for CTFont counts all attributes for font.
            if (CFEqual(fontData->platformData().cgFont(), runCGFont.get())) {
                // This run uses the font we want. Extract glyphs.
                CFIndex glyphCount = CTRunGetGlyphCount(ctRun);
                const CGGlyph* glyphs = CTRunGetGlyphsPtr(ctRun);
                if (!glyphs) {
                    glyphVector.resize(glyphCount);
                    CTRunGetGlyphs(ctRun, CFRangeMake(0, 0), glyphVector.data());
                    glyphs = glyphVector.data();
                }
                const CFIndex* stringIndices = CTRunGetStringIndicesPtr(ctRun);
                if (!stringIndices) {
                    indexVector.resize(glyphCount);
                    CTRunGetStringIndices(ctRun, CFRangeMake(0, 0), indexVector.data());
                    stringIndices = indexVector.data();
                }

                for (CFIndex i = 0; i < glyphCount; ++i) {
                    if (stringIndices[i] >= static_cast<CFIndex>(length)) {
                        done = true;
                        break;
                    }
                    if (glyphs[i]) {
                        setGlyphDataForIndex(offset + stringIndices[i], glyphs[i], fontData);
                        haveGlyphs = true;
                    }
                }
            }
        }
    }
#else
    // Use an array of long so we get good enough alignment.
    long glyphVector[(GLYPH_VECTOR_SIZE + sizeof(long) - 1) / sizeof(long)];
    
    OSStatus status = wkInitializeGlyphVector(GlyphPage::size, &glyphVector);
    if (status != noErr)
        // This should never happen, perhaps indicates a bad font!  If it does the
        // font substitution code will find an alternate font.
        return false;

    wkConvertCharToGlyphs(fontData->m_styleGroup, buffer, bufferLength, &glyphVector);

    unsigned numGlyphs = wkGetGlyphVectorNumGlyphs(&glyphVector);
    if (numGlyphs != length) {
        // This should never happen, perhaps indicates a bad font?
        // If it does happen, the font substitution code will find an alternate font.
        wkClearGlyphVector(&glyphVector);
        return false;
    }

    ATSLayoutRecord* glyphRecord = (ATSLayoutRecord*)wkGetGlyphVectorFirstRecord(glyphVector);
    for (unsigned i = 0; i < length; i++) {
        Glyph glyph = glyphRecord->glyphID;
        if (!glyph)
            setGlyphDataForIndex(offset + i, 0, 0);
        else {
            setGlyphDataForIndex(offset + i, glyph, fontData);
            haveGlyphs = true;
        }
        glyphRecord = (ATSLayoutRecord *)((char *)glyphRecord + wkGetGlyphVectorRecordSize(glyphVector));
    }
    wkClearGlyphVector(&glyphVector);
#endif

    return haveGlyphs;
}

} // namespace WebCore
