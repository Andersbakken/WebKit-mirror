/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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

#include "WebErrors.h"

#include <WebCore/ResourceRequest.h>
#include <WebCore/ResourceResponse.h>

#include <QCoreApplication>
#include <QNetworkReply>

using namespace WebCore;

namespace WebKit {

// copied from WebKit/Misc/WebKitErrors[Private].h
enum {
    WebKitErrorCannotShowMIMEType =                             100,
    WebKitErrorCannotShowURL =                                  101,
    WebKitErrorFrameLoadInterruptedByPolicyChange =             102,
    WebKitErrorCannotUseRestrictedPort = 103,
    WebKitErrorCannotFindPlugIn =                               200,
    WebKitErrorCannotLoadPlugIn =                               201,
    WebKitErrorJavaUnavailable =                                202,
    WebKitErrorPluginWillHandleLoad =                           203
};

ResourceError cancelledError(const ResourceRequest& request)
{
    ResourceError error = ResourceError("QtNetwork", QNetworkReply::OperationCanceledError, request.url().prettyURL(),
                                        QCoreApplication::translate("QWebFrame", "Request cancelled", 0, QCoreApplication::UnicodeUTF8));
    error.setIsCancellation(true);
    return error;    
}

ResourceError blockedError(const ResourceRequest& request)
{
    return ResourceError("WebKit", WebKitErrorCannotUseRestrictedPort, request.url().prettyURL(),
                         QCoreApplication::translate("QWebFrame", "Request blocked", 0, QCoreApplication::UnicodeUTF8));
}

ResourceError cannotShowURLError(const ResourceRequest& request)
{
    return ResourceError("WebKit", WebKitErrorCannotShowURL, request.url().string(),
                         QCoreApplication::translate("QWebFrame", "Cannot show URL", 0, QCoreApplication::UnicodeUTF8));
}

ResourceError interruptForPolicyChangeError(const ResourceRequest& request)
{
    return ResourceError("WebKit", WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(),
                         QCoreApplication::translate("QWebFrame", "Frame load interrupted by policy change", 0, QCoreApplication::UnicodeUTF8));
}

ResourceError cannotShowMIMETypeError(const ResourceResponse& response)
{
    return ResourceError("WebKit", WebKitErrorCannotShowMIMEType, response.url().string(),
                         QCoreApplication::translate("QWebFrame", "Cannot show mimetype", 0, QCoreApplication::UnicodeUTF8));
}

ResourceError fileDoesNotExistError(const ResourceResponse& response)
{
    return ResourceError("QtNetwork", QNetworkReply::ContentNotFoundError, response.url().string(),
                         QCoreApplication::translate("QWebFrame", "File does not exist", 0, QCoreApplication::UnicodeUTF8));
}

ResourceError pluginWillHandleLoadError(const ResourceResponse& response)
{
    return ResourceError("WebKit", WebKitErrorPluginWillHandleLoad, response.url().string(),
                         QCoreApplication::translate("QWebFrame", "Loading is handled by the media engine", 0, QCoreApplication::UnicodeUTF8));
}

} // namespace WebKit
