#ifndef FrameLoaderClientNetflix_H
#define FrameLoaderClientNetflix_H

#include "FrameLoaderClient.h"
#include "StringWithDirection.h"
//#include "FrameLoader.h"
//#include "KURL.h"
#include "ResourceResponse.h"
#include "WebViewNetflix.h"

// copied from WebKit/mac/Misc/WebKitErrors[Private].h
enum {
    WebKitErrorCannotShowMIMEType =                             100,
    WebKitErrorCannotShowURL =                                  101,
    WebKitErrorFrameLoadInterruptedByPolicyChange =             102,
    WebKitErrorCannotUseRestrictedPort =                        103,
    WebKitErrorCannotFindPlugIn =                               200,
    WebKitErrorCannotLoadPlugIn =                               201,
    WebKitErrorJavaUnavailable =                                202,
};

namespace WebKit {

    //class AuthenticationChallenge;
    //class DocumentLoader;
    //class Element;
    //class FormState;
    //class NavigationAction;
    //class String;
    //class ResourceLoader;

    struct LoadErrorResetToken;

    class FrameLoaderClientNetflix : public WebCore::FrameLoaderClient {
    public:
        FrameLoaderClientNetflix();
        ~FrameLoaderClientNetflix();
        void setFrame(WebCore::Frame *frame);
        void setWebView(WebViewNetflix *webView);
        void detachFrameLoader();

        virtual void frameLoaderDestroyed();

        virtual bool hasWebView() const; // mainly for assertions

        virtual void makeRepresentation(WebCore::DocumentLoader*);
        virtual void forceLayout();
        virtual void forceLayoutForNonHTML();

        virtual void setCopiesOnScroll();

        virtual void detachedFromParent2();
        virtual void detachedFromParent3();

        virtual void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&);

        virtual void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long, WebCore::ResourceRequest&, const WebCore::ResourceResponse&);
        virtual bool shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier);
        virtual void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);
        virtual void dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&);
        virtual void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long, const WebCore::ResourceResponse&);
        virtual void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long, int);
        virtual void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long);
        virtual void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long, const WebCore::ResourceError&);
        virtual bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int);

        virtual void dispatchDidHandleOnloadEvents();
        virtual void dispatchDidReceiveServerRedirectForProvisionalLoad();
        virtual void dispatchDidCancelClientRedirect();
        virtual void dispatchWillPerformClientRedirect(const WebCore::KURL&, double interval, double fireDate);
        virtual void dispatchDidChangeLocationWithinPage();
        virtual void dispatchDidPushStateWithinPage() { }
        virtual void dispatchDidReplaceStateWithinPage() { }
        virtual void dispatchDidPopStateWithinPage() { }
        virtual void dispatchWillClose();
        virtual void dispatchDidReceiveIcon();
        virtual void dispatchDidStartProvisionalLoad();
        virtual void dispatchDidReceiveTitle(const WebCore::StringWithDirection&);
        virtual void dispatchDidChangeIcons(WebCore::IconType) { }
        virtual void dispatchDidCommitLoad();
        virtual void dispatchDidFailProvisionalLoad(const WebCore::ResourceError & error);
        virtual void dispatchDidFailLoad(const WebCore::ResourceError & error);
        virtual void dispatchDidFinishDocumentLoad();
        virtual void dispatchDidFinishLoad();
        virtual void dispatchDidFirstLayout();
        virtual void dispatchDidFirstVisuallyNonEmptyLayout();

        virtual WebCore::Frame* dispatchCreatePage(const WebCore::NavigationAction&);
        virtual void dispatchShow();

        virtual void dispatchDecidePolicyForResponse(WebCore::FramePolicyFunction, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&);
        virtual void dispatchDecidePolicyForMIMEType(WebCore::FramePolicyFunction function, const WTF::String&, const WebCore::ResourceRequest&);
        virtual void dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&,
                                                            const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>, const WTF::String&);
        virtual void dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&,
                                                             const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>);
        virtual void cancelPolicyCheck();

        virtual bool shouldStopLoadingForHistoryItem(WebCore::HistoryItem*) const { return false; }

        virtual void dispatchUnableToImplementPolicy(const WebCore::ResourceError&);

        virtual void dispatchWillSendSubmitEvent(WebCore::HTMLFormElement*) { }
        virtual void dispatchWillSubmitForm(WebCore::FramePolicyFunction, PassRefPtr<WebCore::FormState>);

        virtual void dispatchDidLoadMainResource(WebCore::DocumentLoader*);
        virtual void revertToProvisionalState(WebCore::DocumentLoader*);
        virtual void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&);

        virtual void postProgressStartedNotification();
        virtual void postProgressEstimateChangedNotification();
        virtual void postProgressFinishedNotification();

        virtual void setMainFrameDocumentReady(bool);

        virtual void startDownload(const WebCore::ResourceRequest&, const WTF::String &);

        virtual void willChangeTitle(WebCore::DocumentLoader*);
        virtual void didChangeTitle(WebCore::DocumentLoader*);

        virtual void committedLoad(WebCore::DocumentLoader*, const char*, int);
        virtual void finishedLoading(WebCore::DocumentLoader*);

        virtual void updateGlobalHistory();
        virtual void updateGlobalHistoryRedirectLinks();

        virtual bool shouldGoToHistoryItem(WebCore::HistoryItem*) const;
        virtual void dispatchDidAddBackForwardItem(WebCore::HistoryItem*) const { }
        virtual void dispatchDidRemoveBackForwardItem(WebCore::HistoryItem*) const { }
        virtual void dispatchDidChangeBackForwardIndex() const { }
        virtual void dispatchDidBecomeFrameset(bool) { }


        virtual WebCore::ResourceError interruptedForPolicyChangeError(const WebCore::ResourceRequest&);

        virtual void didDisplayInsecureContent();

        virtual void didRunInsecureContent(WebCore::SecurityOrigin*, const WebCore::KURL &);

        virtual WebCore::ResourceError cancelledError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError cannotShowURLError(const WebCore::ResourceRequest&);
        virtual WebCore::ResourceError interruptForPolicyChangeError(const WebCore::ResourceRequest&);

        virtual WebCore::ResourceError cannotShowMIMETypeError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError fileDoesNotExistError(const WebCore::ResourceResponse&);
        virtual WebCore::ResourceError pluginWillHandleLoadError(const WebCore::ResourceResponse&);

        virtual bool shouldFallBack(const WebCore::ResourceError&);

        virtual bool canHandleRequest(const WebCore::ResourceRequest&) const;
        virtual bool canShowMIMEType(const WTF::String& MIMEType) const;
        virtual bool canShowMIMETypeAsHTML(const WTF::String& MIMEType) const;
        virtual bool representationExistsForURLScheme(const WTF::String& URLScheme) const;
        virtual WTF::String generatedMIMETypeForURLScheme(const WTF::String& URLScheme) const;

        virtual void didSaveToPageCache();
        virtual void didRestoreFromPageCache();

        virtual void frameLoadCompleted();
        virtual void saveViewStateToItem(WebCore::HistoryItem*);
        virtual void restoreViewState();
        virtual void provisionalLoadStarted();
        virtual void didFinishLoad();
        virtual void prepareForDataSourceReplacement();

        virtual WTF::PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&);
        virtual void setTitle(const WebCore::StringWithDirection& title, const WebCore::KURL&);

        virtual WTF::String userAgent(const WebCore::KURL&);

        virtual void savePlatformDataToCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*);
        virtual void transitionToCommittedForNewPage();

        virtual bool canCachePage() const;
        virtual void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&);

        virtual void transferLoadingResourceFromPage(long unsigned int, WebCore::DocumentLoader*, const WebCore::ResourceRequest&, WebCore::Page*);

        virtual PassRefPtr<WebCore::Frame> createFrame(const WebCore::KURL& url, const WTF::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                   const WTF::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight);
        virtual void didTransferChildFrameToNewDocument(WebCore::Page *) { }
        virtual void transferLoadingResourceFromPage(WebCore::ResourceLoader*, const WebCore::ResourceRequest&, WebCore::Page* oldPage) { }
        virtual PassRefPtr<WebCore::Widget> createPlugin(const WebCore::IntSize&, WebCore::HTMLPlugInElement*, const WebCore::KURL&,
                                                         const Vector<WTF::String>&, const Vector<WTF::String>&, const WTF::String&, bool loadManually) ;
        virtual void redirectDataToPlugin(WebCore::Widget* pluginWidget);

        virtual PassRefPtr<WebCore::Widget> createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL,
                                                                   const Vector<WTF::String>& paramNames, const Vector<WTF::String>& paramValues);

        virtual WebCore::ObjectContentType objectContentType(const WebCore::KURL& url, const WTF::String& mimeType, bool);
        virtual WTF::String overrideMediaType() const;

        virtual WTF::PassRefPtr<WebCore::FrameNetworkingContext> createNetworkingContext();

        virtual void dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld*);
        virtual void documentElementAvailable();
        virtual void didPerformFirstNavigation() const;

        virtual void registerForIconNotification(bool listen = true);

    private:
        WebCore::Frame *m_frame;
        WebViewNetflix *m_webView;
        WebCore::ResourceResponse m_response;
        bool m_firstData;
    };

}

#endif
