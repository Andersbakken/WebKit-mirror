/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef WebIDBDatabaseError_h
#define WebIDBDatabaseError_h

#include "WebCommon.h"
#include "WebPrivatePtr.h"
#include "WebString.h"

namespace WebCore { class IDBDatabaseError; }

namespace WebKit {

// See comment in WebIDBFactory for a high level overview these classes.
class WebIDBDatabaseError {
public:
    ~WebIDBDatabaseError() { reset(); }

    WEBKIT_API WebIDBDatabaseError(unsigned short code, const WebString& message) { assign(code, message); }
    WEBKIT_API WebIDBDatabaseError(const WebIDBDatabaseError& e) { assign(e); }
    WEBKIT_API WebIDBDatabaseError& operator=(const WebIDBDatabaseError& e)
    {
        assign(e);
        return *this;
    }

    WEBKIT_API void assign(const WebIDBDatabaseError&);
    WEBKIT_API void reset();

    WEBKIT_API unsigned short code() const;
    WEBKIT_API WebString message() const;

#if WEBKIT_IMPLEMENTATION
    WebIDBDatabaseError(const WTF::PassRefPtr<WebCore::IDBDatabaseError>&);
    WebIDBDatabaseError& operator=(const WTF::PassRefPtr<WebCore::IDBDatabaseError>&);
    operator WTF::PassRefPtr<WebCore::IDBDatabaseError>() const;
#endif

private:
    WEBKIT_API void assign(unsigned short code, const WebString& message);

    WebPrivatePtr<WebCore::IDBDatabaseError> m_private;
};

} // namespace WebKit

#endif // WebIDBDatabaseError_h
