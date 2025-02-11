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

#ifndef WebContext_h
#define WebContext_h

#include "APIObject.h"
#include "PluginInfoStore.h"
#include "ProcessModel.h"
#include "VisitedLinkProvider.h"
#include "WebContextInjectedBundleClient.h"
#include "WebDownloadClient.h"
#include "WebHistoryClient.h"
#include "WebProcessProxy.h"
#include <WebCore/LinkHash.h>
#include <wtf/Forward.h>
#include <wtf/HashSet.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

namespace WebKit {

class DownloadProxy;
class WebDatabaseManagerProxy;
class WebPageGroup;
class WebPageProxy;
struct WebProcessCreationParameters;

class WebContext : public APIObject {
public:
    static const Type APIType = TypeContext;

    static WebContext* sharedProcessContext();
    static WebContext* sharedThreadContext();

    static PassRefPtr<WebContext> create(const String& injectedBundlePath);
    virtual ~WebContext();

    void initializeInjectedBundleClient(const WKContextInjectedBundleClient*);
    void initializeHistoryClient(const WKContextHistoryClient*);
    void initializeDownloadClient(const WKContextDownloadClient*);

    ProcessModel processModel() const { return m_processModel; }
    WebProcessProxy* process() const { return m_process.get(); }

    void processDidFinishLaunching(WebProcessProxy*);
    void processDidClose(WebProcessProxy*);

    WebPageProxy* createWebPage(WebPageGroup* = 0);

    void relaunchProcessIfNecessary();

    const String& injectedBundlePath() const { return m_injectedBundlePath; }

    void setInjectedBundleInitializationUserData(PassRefPtr<APIObject> userData) { m_injectedBundleInitializationUserData = userData; }
    APIObject* injectedBundleInitializationUserData() const { return m_injectedBundleInitializationUserData.get(); }

    void postMessageToInjectedBundle(const String&, APIObject*);

    // InjectedBundle client
    void didReceiveMessageFromInjectedBundle(const String&, APIObject*);
    void didReceiveSynchronousMessageFromInjectedBundle(const String&, APIObject*, RefPtr<APIObject>& returnData);

    void populateVisitedLinks();
    
    void setAdditionalPluginsDirectory(const String&);

    PluginInfoStore* pluginInfoStore() { return &m_pluginInfoStore; }
    String applicationCacheDirectory();

    void setAlwaysUsesComplexTextCodePath(bool);
    
    void registerURLSchemeAsEmptyDocument(const String&);
    void registerURLSchemeAsSecure(const String&);
    void setDomainRelaxationForbiddenForURLScheme(const String&);

    void addVisitedLink(const String&);
    void addVisitedLinkHash(WebCore::LinkHash);

    void didReceiveMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    CoreIPC::SyncReplyMode didReceiveSyncMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*, CoreIPC::ArgumentEncoder*);

    void setCacheModel(CacheModel);
    CacheModel cacheModel() const { return m_cacheModel; }
    void clearResourceCaches();
    void clearApplicationCache();

#if PLATFORM(WIN)
    void setShouldPaintNativeControls(bool);
#endif

    // Downloads.
    uint64_t createDownloadProxy();
    WebDownloadClient& downloadClient() { return m_downloadClient; }
    void downloadFinished(DownloadProxy*);

    static HashSet<String, CaseFoldingHash> pdfAndPostScriptMIMETypes();

    WebDatabaseManagerProxy* databaseManagerProxy() const { return m_databaseManagerProxy.get(); }

    struct Statistics {
        unsigned wkViewCount;
        unsigned wkPageCount;
        unsigned wkFrameCount;
    };
    static Statistics& statistics();

private:
    WebContext(ProcessModel, const String& injectedBundlePath);

    virtual Type type() const { return APIType; }

    void ensureWebProcess();
    bool hasValidProcess() const { return m_process && m_process->isValid(); }
    void platformInitializeWebProcess(WebProcessCreationParameters&);

    // History client
    void didNavigateWithNavigationData(uint64_t pageID, const WebNavigationDataStore& store, uint64_t frameID);
    void didPerformClientRedirect(uint64_t pageID, const String& sourceURLString, const String& destinationURLString, uint64_t frameID);
    void didPerformServerRedirect(uint64_t pageID, const String& sourceURLString, const String& destinationURLString, uint64_t frameID);
    void didUpdateHistoryTitle(uint64_t pageID, const String& title, const String& url, uint64_t frameID);

    // Plugins
    void getPlugins(bool refresh, Vector<WebCore::PluginInfo>& plugins);
    void getPluginPath(const String& mimeType, const String& urlString, String& pluginPath);

    // Implemented in generated WebContextMessageReceiver.cpp
    void didReceiveWebContextMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    CoreIPC::SyncReplyMode didReceiveSyncWebContextMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*, CoreIPC::ArgumentEncoder*);

    static void languageChanged(void* context);
    void languageChanged();

    ProcessModel m_processModel;
    
    // FIXME: In the future, this should be one or more WebProcessProxies.
    RefPtr<WebProcessProxy> m_process;

    RefPtr<WebPageGroup> m_defaultPageGroup;

    RefPtr<APIObject> m_injectedBundleInitializationUserData;
    String m_injectedBundlePath;
    WebContextInjectedBundleClient m_injectedBundleClient;

    WebHistoryClient m_historyClient;

    PluginInfoStore m_pluginInfoStore;
    VisitedLinkProvider m_visitedLinkProvider;
        
    HashSet<String> m_schemesToRegisterAsEmptyDocument;
    HashSet<String> m_schemesToRegisterAsSecure;
    HashSet<String> m_schemesToSetDomainRelaxationForbiddenFor;

    bool m_alwaysUsesComplexTextCodePath;

    Vector<pair<String, RefPtr<APIObject> > > m_pendingMessagesToPostToInjectedBundle;

    CacheModel m_cacheModel;

    WebDownloadClient m_downloadClient;
    HashMap<uint64_t, RefPtr<DownloadProxy> > m_downloads;

    bool m_clearResourceCachesForNewWebProcess;
    bool m_clearApplicationCacheForNewWebProcess;

    RefPtr<WebDatabaseManagerProxy> m_databaseManagerProxy;

#if PLATFORM(WIN)
    bool m_shouldPaintNativeControls;
#endif
};

} // namespace WebKit

#endif // WebContext_h
