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

#ifndef Download_h
#define Download_h

#include "MessageSender.h"
#include <WebCore/ResourceRequest.h>
#include <wtf/Noncopyable.h>
#include <wtf/PassOwnPtr.h>

#if PLATFORM(MAC)
#include <wtf/RetainPtr.h>
#ifdef __OBJC__
@class NSURLDownload;
@class WKDownloadAsDelegate;
#else
class NSURLDownload;
class WKDownloadAsDelegate;
#endif
#endif

#if USE(CFNETWORK)
#include <CFNetwork/CFURLDownloadPriv.h>
#endif

namespace CoreIPC {
    class DataReference;
}

namespace WebCore {
    class ResourceError;
    class ResourceHandle;
    class ResourceResponse;
}

namespace WebKit {

class SandboxExtension;
class WebPage;

class Download : public CoreIPC::MessageSender<Download> {
    WTF_MAKE_NONCOPYABLE(Download);

public:
    static PassOwnPtr<Download> create(uint64_t downloadID, const WebCore::ResourceRequest&);
    ~Download();

    // Used by MessageSender.
    CoreIPC::Connection* connection() const;
    uint64_t destinationID() const { return downloadID(); }

    void start(WebPage* initiatingWebPage);
    void startWithHandle(WebPage* initiatingPage, WebCore::ResourceHandle*, const WebCore::ResourceRequest& initialRequest, const WebCore::ResourceResponse&);
    void cancel();

    uint64_t downloadID() const { return m_downloadID; }

    void didStart();
    void didReceiveResponse(const WebCore::ResourceResponse&);
    void didReceiveData(uint64_t length);
    bool shouldDecodeSourceDataOfMIMEType(const String& mimeType);
    String decideDestinationWithSuggestedFilename(const String& filename, bool& allowOverwrite);
    void didCreateDestination(const String& path);
    void didFinish();
    void didFail(const WebCore::ResourceError&, const CoreIPC::DataReference& resumeData);
    void didCancel(const CoreIPC::DataReference& resumeData);

private:
    Download(uint64_t downloadID, const WebCore::ResourceRequest&);

    void platformInvalidate();

    uint64_t m_downloadID;
    WebCore::ResourceRequest m_request;

    RefPtr<SandboxExtension> m_sandboxExtension;

#if PLATFORM(MAC)
    RetainPtr<NSURLDownload> m_nsURLDownload;
    RetainPtr<WKDownloadAsDelegate> m_delegate;
#endif
#if USE(CFNETWORK)
    RetainPtr<CFURLDownloadRef> m_download;
#endif
};

} // namespace WebKit

#endif // Download_h
