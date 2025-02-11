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

#include "WebProcessCreationParameters.h"

#include "ArgumentCoders.h"

namespace WebKit {

WebProcessCreationParameters::WebProcessCreationParameters()
    : shouldTrackVisitedLinks(false)
    , shouldAlwaysUseComplexTextCodePath(false)
#if PLATFORM(MAC)
    , nsURLCacheMemoryCapacity(0)
    , nsURLCacheDiskCapacity(0)
#elif PLATFORM(WIN)
    , shouldPaintNativeControls(false)
#endif
{
}

void WebProcessCreationParameters::encode(CoreIPC::ArgumentEncoder* encoder) const
{
    encoder->encode(injectedBundlePath);
    encoder->encode(injectedBundlePathExtensionHandle);
    encoder->encode(applicationCacheDirectory);
    encoder->encode(urlSchemesRegistererdAsEmptyDocument);
    encoder->encode(urlSchemesRegisteredAsSecure);
    encoder->encode(urlSchemesForWhichDomainRelaxationIsForbidden);
    encoder->encode(mimeTypesWithCustomRepresentation);
    encoder->encodeEnum(cacheModel);
    encoder->encode(shouldTrackVisitedLinks);
    encoder->encode(shouldAlwaysUseComplexTextCodePath);
    encoder->encode(languageCode);
#if PLATFORM(MAC)
    encoder->encode(nsURLCachePath);
    encoder->encode(nsURLCacheMemoryCapacity);
    encoder->encode(nsURLCacheDiskCapacity);
    encoder->encode(acceleratedCompositingPort);
#elif PLATFORM(WIN)
    encoder->encode(shouldPaintNativeControls);
#endif
}

bool WebProcessCreationParameters::decode(CoreIPC::ArgumentDecoder* decoder, WebProcessCreationParameters& parameters)
{
    if (!decoder->decode(parameters.injectedBundlePath))
        return false;
    if (!decoder->decode(parameters.injectedBundlePathExtensionHandle))
        return false;
    if (!decoder->decode(parameters.applicationCacheDirectory))
        return false;
    if (!decoder->decode(parameters.urlSchemesRegistererdAsEmptyDocument))
        return false;
    if (!decoder->decode(parameters.urlSchemesRegisteredAsSecure))
        return false;
    if (!decoder->decode(parameters.urlSchemesForWhichDomainRelaxationIsForbidden))
        return false;
    if (!decoder->decode(parameters.mimeTypesWithCustomRepresentation))
        return false;
    if (!decoder->decodeEnum(parameters.cacheModel))
        return false;
    if (!decoder->decode(parameters.shouldTrackVisitedLinks))
        return false;
    if (!decoder->decode(parameters.shouldAlwaysUseComplexTextCodePath))
        return false;
    if (!decoder->decode(parameters.languageCode))
        return false;
#if PLATFORM(MAC)
    if (!decoder->decode(parameters.nsURLCachePath))
        return false;
    if (!decoder->decode(parameters.nsURLCacheMemoryCapacity))
        return false;
    if (!decoder->decode(parameters.nsURLCacheDiskCapacity))
        return false;
    if (!decoder->decode(parameters.acceleratedCompositingPort))
        return false;
#elif PLATFORM(WIN)
    if (!decoder->decode(parameters.shouldPaintNativeControls))
        return false;
#endif

    return true;
}

} // namespace WebKit
