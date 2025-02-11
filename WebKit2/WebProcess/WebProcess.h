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

#ifndef WebProcess_h
#define WebProcess_h

#include "CacheModel.h"
#include "ChildProcess.h"
#include "DrawingArea.h"
#include "SharedMemory.h"
#include "VisitedLinkTable.h"
#include "WebPageGroupProxy.h"
#include <WebCore/LinkHash.h>
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>

#if PLATFORM(MAC)
#include "MachPort.h"
#endif

#if PLATFORM(QT)
class QNetworkAccessManager;
#endif

namespace WebCore {
    class IntSize;
    class PageGroup;
}

namespace WebKit {

class InjectedBundle;
class WebFrame;
class WebPage;
struct WebPageCreationParameters;
struct WebPageGroupData;
struct WebPreferencesStore;
struct WebProcessCreationParameters;

class WebProcess : ChildProcess {
public:
    static WebProcess& shared();

    void initialize(CoreIPC::Connection::Identifier, RunLoop* runLoop);

    CoreIPC::Connection* connection() const { return m_connection.get(); }
    RunLoop* runLoop() const { return m_runLoop; }

    WebPage* webPage(uint64_t pageID) const;
    void createWebPage(uint64_t pageID, const WebPageCreationParameters&);
    void removeWebPage(uint64_t pageID);

    InjectedBundle* injectedBundle() const { return m_injectedBundle.get(); }

    bool isSeparateProcess() const;

#if USE(ACCELERATED_COMPOSITING) && PLATFORM(MAC)
    mach_port_t compositingRenderServerPort() const { return m_compositingRenderServerPort; }
#endif
    
    void addVisitedLink(WebCore::LinkHash);
    bool isLinkVisited(WebCore::LinkHash) const;

    WebFrame* webFrame(uint64_t) const;
    void addWebFrame(uint64_t, WebFrame*);
    void removeWebFrame(uint64_t);

    WebPageGroupProxy* webPageGroup(uint64_t pageGroupID);
    WebPageGroupProxy* webPageGroup(const WebPageGroupData&);
    static WebCore::PageGroup* sharedPageGroup();

#if PLATFORM(QT)
    QNetworkAccessManager* networkAccessManager() { return m_networkAccessManager; }
#endif

    // Will shut down the web process if there are no live pages or downloads.
    void shutdownIfPossible();

    bool shouldUseCustomRepresentationForMIMEType(const String& mimeType) const { return m_mimeTypesWithCustomRepresentations.contains(mimeType); }

private:
    WebProcess();

    void initializeWebProcess(const WebProcessCreationParameters&, CoreIPC::ArgumentDecoder*);
    void platformInitializeWebProcess(const WebProcessCreationParameters&, CoreIPC::ArgumentDecoder*);
    void platformShutdown();
    void setShouldTrackVisitedLinks(bool);
    void registerURLSchemeAsEmptyDocument(const String&);
    void registerURLSchemeAsSecure(const String&) const;
    void setDomainRelaxationForbiddenForURLScheme(const String&) const;
    void setAlwaysUsesComplexTextCodePath(bool);
    void languageChanged(const String&) const;
#if PLATFORM(WIN)
    void setShouldPaintNativeControls(bool);
#endif

    void setVisitedLinkTable(const SharedMemory::Handle&);
    void visitedLinkStateChanged(const Vector<WebCore::LinkHash>& linkHashes);
    void allVisitedLinkStateChanged();

    void setCacheModel(uint32_t);
    void platformSetCacheModel(CacheModel);
    static void calculateCacheSizes(CacheModel cacheModel, uint64_t memorySize, uint64_t diskFreeSize,
        unsigned& cacheTotalCapacity, unsigned& cacheMinDeadCapacity, unsigned& cacheMaxDeadCapacity, double& deadDecodedDataDeletionInterval,
        unsigned& pageCacheCapacity, unsigned long& urlCacheMemoryCapacity, unsigned long& urlCacheDiskCapacity);
    void clearResourceCaches();
    void platformClearResourceCaches();
    void clearApplicationCache();

    void cancelDownload(uint64_t downloadID);

    // CoreIPC::Connection::Client
    void didReceiveMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    CoreIPC::SyncReplyMode didReceiveSyncMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*, CoreIPC::ArgumentEncoder*);
    void didClose(CoreIPC::Connection*);
    void didReceiveInvalidMessage(CoreIPC::Connection*, CoreIPC::MessageID);

    // Implemented in generated WebProcessMessageReceiver.cpp
    void didReceiveWebProcessMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    
    RefPtr<CoreIPC::Connection> m_connection;
    HashMap<uint64_t, RefPtr<WebPage> > m_pageMap;
    HashMap<uint64_t, RefPtr<WebPageGroupProxy> > m_pageGroupMap;
    RefPtr<InjectedBundle> m_injectedBundle;

    bool m_inDidClose;

    RunLoop* m_runLoop;

    // FIXME: The visited link table should not be per process.
    VisitedLinkTable m_visitedLinkTable;

    bool m_hasSetCacheModel;
    CacheModel m_cacheModel;

#if USE(ACCELERATED_COMPOSITING) && PLATFORM(MAC)
    mach_port_t m_compositingRenderServerPort;
#endif

#if PLATFORM(QT)
    QNetworkAccessManager* m_networkAccessManager;
#endif

    HashMap<uint64_t, WebFrame*> m_frameMap;

    HashSet<String, CaseFoldingHash> m_mimeTypesWithCustomRepresentations;
};

} // namespace WebKit

#endif // WebProcess_h
