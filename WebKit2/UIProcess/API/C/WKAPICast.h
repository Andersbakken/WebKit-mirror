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

#ifndef WKAPICast_h
#define WKAPICast_h

#include "CacheModel.h"
#include "FontSmoothingLevel.h"
#include "WKContext.h"
#include "WKCredentialTypes.h"
#include "WKPage.h"
#include "WKPreferencesPrivate.h"
#include "WKProtectionSpaceTypes.h"
#include "WKSharedAPICast.h"
#include <WebCore/Credential.h>
#include <WebCore/FrameLoaderTypes.h>
#include <WebCore/ProtectionSpace.h>

namespace WebKit {

class AuthenticationChallengeProxy;
class AuthenticationDecisionListener;
class DownloadProxy;
class WebBackForwardList;
class WebBackForwardListItem;
class WebContext;
class WebCredential;
class WebDatabaseManagerProxy;
class WebFormSubmissionListenerProxy;
class WebFramePolicyListenerProxy;
class WebFrameProxy;
class WebInspectorProxy;
class WebNavigationData;
class WebOpenPanelParameters;
class WebOpenPanelResultListenerProxy;
class WebPageGroup;
class WebPageProxy;
class WebPreferences;
class WebProtectionSpace;

WK_ADD_API_MAPPING(WKAuthenticationChallengeRef, AuthenticationChallengeProxy)
WK_ADD_API_MAPPING(WKAuthenticationDecisionListenerRef, AuthenticationDecisionListener)
WK_ADD_API_MAPPING(WKBackForwardListItemRef, WebBackForwardListItem)
WK_ADD_API_MAPPING(WKBackForwardListRef, WebBackForwardList)
WK_ADD_API_MAPPING(WKContextRef, WebContext)
WK_ADD_API_MAPPING(WKCredentialRef, WebCredential)
WK_ADD_API_MAPPING(WKDatabaseManagerRef, WebDatabaseManagerProxy)
WK_ADD_API_MAPPING(WKDownloadRef, DownloadProxy)
WK_ADD_API_MAPPING(WKFormSubmissionListenerRef, WebFormSubmissionListenerProxy)
WK_ADD_API_MAPPING(WKFramePolicyListenerRef, WebFramePolicyListenerProxy)
WK_ADD_API_MAPPING(WKFrameRef, WebFrameProxy)
WK_ADD_API_MAPPING(WKNavigationDataRef, WebNavigationData)
WK_ADD_API_MAPPING(WKOpenPanelParametersRef, WebOpenPanelParameters)
WK_ADD_API_MAPPING(WKOpenPanelResultListenerRef, WebOpenPanelResultListenerProxy)
WK_ADD_API_MAPPING(WKPageGroupRef, WebPageGroup)
WK_ADD_API_MAPPING(WKPageRef, WebPageProxy)
WK_ADD_API_MAPPING(WKPreferencesRef, WebPreferences)
WK_ADD_API_MAPPING(WKProtectionSpaceRef, WebProtectionSpace)

#if ENABLE(INSPECTOR)
WK_ADD_API_MAPPING(WKInspectorRef, WebInspectorProxy)
#endif

/* Enum conversions */

inline CacheModel toCacheModel(WKCacheModel wkCacheModel)
{
    switch (wkCacheModel) {
    case kWKCacheModelDocumentViewer:
        return CacheModelDocumentViewer;
    case kWKCacheModelDocumentBrowser:
        return CacheModelDocumentBrowser;
    case kWKCacheModelPrimaryWebBrowser:
        return CacheModelPrimaryWebBrowser;
    }

    ASSERT_NOT_REACHED();
    return CacheModelDocumentViewer;
}

inline WKCacheModel toAPI(CacheModel cacheModel)
{
    switch (cacheModel) {
    case CacheModelDocumentViewer:
        return kWKCacheModelDocumentViewer;
    case CacheModelDocumentBrowser:
        return kWKCacheModelDocumentBrowser;
    case CacheModelPrimaryWebBrowser:
        return kWKCacheModelPrimaryWebBrowser;
    }
    
    return kWKCacheModelDocumentViewer;
}

inline FontSmoothingLevel toFontSmoothingLevel(WKFontSmoothingLevel wkLevel)
{
    switch (wkLevel) {
    case kWKFontSmoothingLevelNoSubpixelAntiAliasing:
        return FontSmoothingLevelNoSubpixelAntiAliasing;
    case kWKFontSmoothingLevelLight:
        return FontSmoothingLevelLight;
    case kWKFontSmoothingLevelMedium:
        return FontSmoothingLevelMedium;
    case kWKFontSmoothingLevelStrong:
        return FontSmoothingLevelStrong;
#if PLATFORM(WIN)
    case kWKFontSmoothingLevelWindows:
        return FontSmoothingLevelWindows;
#endif
    }

    ASSERT_NOT_REACHED();
    return FontSmoothingLevelMedium;
}


inline WKFontSmoothingLevel toAPI(FontSmoothingLevel level)
{
    switch (level) {
    case FontSmoothingLevelNoSubpixelAntiAliasing:
        return kWKFontSmoothingLevelNoSubpixelAntiAliasing;
    case FontSmoothingLevelLight:
        return kWKFontSmoothingLevelLight;
    case FontSmoothingLevelMedium:
        return kWKFontSmoothingLevelMedium;
    case FontSmoothingLevelStrong:
        return kWKFontSmoothingLevelStrong;
#if PLATFORM(WIN)
    case FontSmoothingLevelWindows:
        return kWKFontSmoothingLevelWindows;
#endif
    }

    ASSERT_NOT_REACHED();
    return kWKFontSmoothingLevelMedium;
}

inline WKProtectionSpaceServerType toAPI(WebCore::ProtectionSpaceServerType type)
{
    switch (type) {
    case WebCore::ProtectionSpaceServerHTTP:
        return kWKProtectionSpaceServerTypeHTTP;
    case WebCore::ProtectionSpaceServerHTTPS:
        return kWKProtectionSpaceServerTypeHTTPS;
    case WebCore::ProtectionSpaceServerFTP:
        return kWKProtectionSpaceServerTypeFTP;
    case WebCore::ProtectionSpaceServerFTPS:
        return kWKProtectionSpaceServerTypeFTPS;
    case WebCore::ProtectionSpaceProxyHTTP:
        return kWKProtectionSpaceProxyTypeHTTP;
    case WebCore::ProtectionSpaceProxyHTTPS:
        return kWKProtectionSpaceProxyTypeHTTPS;
    case WebCore::ProtectionSpaceProxyFTP:
        return kWKProtectionSpaceProxyTypeFTP;
    case WebCore::ProtectionSpaceProxySOCKS:
        return kWKProtectionSpaceProxyTypeSOCKS;
    }
    return kWKProtectionSpaceServerTypeHTTP;
}

inline WKProtectionSpaceAuthenticationScheme toAPI(WebCore::ProtectionSpaceAuthenticationScheme type)
{
    switch (type) {
    case WebCore::ProtectionSpaceAuthenticationSchemeDefault:
        return kWKProtectionSpaceAuthenticationSchemeDefault;
    case WebCore::ProtectionSpaceAuthenticationSchemeHTTPBasic:
        return kWKProtectionSpaceAuthenticationSchemeHTTPBasic;
    case WebCore::ProtectionSpaceAuthenticationSchemeHTTPDigest:
        return kWKProtectionSpaceAuthenticationSchemeHTTPDigest;
    case WebCore::ProtectionSpaceAuthenticationSchemeHTMLForm:
        return kWKProtectionSpaceAuthenticationSchemeHTMLForm;
    case WebCore::ProtectionSpaceAuthenticationSchemeNTLM:
        return kWKProtectionSpaceAuthenticationSchemeNTLM;
    case WebCore::ProtectionSpaceAuthenticationSchemeNegotiate:
        return kWKProtectionSpaceAuthenticationSchemeNegotiate;
    case WebCore::ProtectionSpaceAuthenticationSchemeClientCertificateRequested:
        return kWKProtectionSpaceAuthenticationSchemeClientCertificateRequested;
    case WebCore::ProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested:
        return kWKProtectionSpaceAuthenticationSchemeServerTrustEvaluationRequested;
    default:
        return kWKProtectionSpaceAuthenticationSchemeUnknown;
    }
}

inline WebCore::CredentialPersistence toCredentialPersistence(WKCredentialPersistence type)
{
    switch (type) {
    case kWKCredentialPersistenceNone:
        return WebCore::CredentialPersistenceNone;
    case kWKCredentialPersistenceForSession:
        return WebCore::CredentialPersistenceForSession;
    case kWKCredentialPersistencePermanent:
        return WebCore::CredentialPersistencePermanent;
    default:
        return WebCore::CredentialPersistenceNone;
    }
}

} // namespace WebKit

#if defined(WIN32) || defined(_WIN32)
#include "WKAPICastWin.h"
#endif

#endif // WKAPICast_h
