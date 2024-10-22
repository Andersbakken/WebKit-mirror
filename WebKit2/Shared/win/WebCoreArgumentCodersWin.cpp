/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebCoreArgumentCoders.h"

#if USE(CFNETWORK)
#include "ArgumentCodersCF.h"
#include <WebKitSystemInterface/WebKitSystemInterface.h>
#endif

namespace CoreIPC {

// FIXME: These coders should really go in a WebCoreArgumentCodersCFNetwork file.

void encodeResourceRequest(ArgumentEncoder* encoder, const WebCore::ResourceRequest& resourceRequest)
{
#if USE(CFNETWORK)
    RetainPtr<CFDictionaryRef> dictionary(AdoptCF, wkCFURLRequestCreateSerializableRepresentation(resourceRequest.cfURLRequest(), CoreIPC::tokenNullTypeRef()));
    encode(encoder, dictionary.get());
#endif
}

bool decodeResourceRequest(ArgumentDecoder* decoder, WebCore::ResourceRequest& resourceRequest)
{
#if USE(CFNETWORK)
    RetainPtr<CFDictionaryRef> dictionary;
    if (!decode(decoder, dictionary))
        return false;

    CFURLRequestRef cfURLRequest = wkCFURLRequestCreateFromSerializableRepresentation(dictionary.get(), CoreIPC::tokenNullTypeRef());
    if (!cfURLRequest)
        return false;

    resourceRequest = WebCore::ResourceRequest(cfURLRequest);
    return true;
#else
    return false;
#endif
}

void encodeResourceResponse(ArgumentEncoder* encoder, const WebCore::ResourceResponse& resourceResponse)
{
#if USE(CFNETWORK)
    bool responseIsPresent = resourceResponse.cfURLResponse();
    encoder->encode(responseIsPresent);

    if (!responseIsPresent)
        return;

    RetainPtr<CFDictionaryRef> dictionary(AdoptCF, wkCFURLResponseCreateSerializableRepresentation(resourceResponse.cfURLResponse(), CoreIPC::tokenNullTypeRef()));
    encode(encoder, dictionary.get());
#endif
}

bool decodeResourceResponse(ArgumentDecoder* decoder, WebCore::ResourceResponse& resourceResponse)
{
#if USE(CFNETWORK)
    bool responseIsPresent;
    decoder->decode(responseIsPresent);

    if (!responseIsPresent) {
        resourceResponse = WebCore::ResourceResponse();
        return true;
    }

    RetainPtr<CFDictionaryRef> dictionary;
    if (!decode(decoder, dictionary))
        return false;

    CFURLResponseRef cfURLResponse = wkCFURLResponseCreateFromSerializableRepresentation(dictionary.get(), CoreIPC::tokenNullTypeRef());
    if (!cfURLResponse)
        return false;

    resourceResponse = WebCore::ResourceResponse(cfURLResponse);
    return true;
#else
    return false;
#endif
}

} // namespace CoreIPC
