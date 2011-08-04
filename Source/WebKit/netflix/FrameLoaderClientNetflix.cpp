#include "config.h"
#include "FrameLoaderClientNetflix.h"
#include "FrameNetworkingContextNetflix.h"

#include "DocumentLoader.h"
#include "Frame.h"
#include "FrameView.h"
#include "HTMLFrameOwnerElement.h"
#include "KURL.h"
#include "NotImplemented.h"
#include "Page.h"
#include "Widget.h"

using namespace WebCore;

namespace WebKit {

FrameLoaderClientNetflix::FrameLoaderClientNetflix()
    : m_frame(0)
{
}

FrameLoaderClientNetflix::~FrameLoaderClientNetflix()
{
}

void FrameLoaderClientNetflix::setFrame(Frame *frame)
{
    m_frame = frame;
}

void FrameLoaderClientNetflix::setWebView(WebViewNetflix *webview)
{
    m_webView = webview;
}

void FrameLoaderClientNetflix::detachFrameLoader()
{
    m_frame = 0;
}

bool FrameLoaderClientNetflix::hasWebView() const
{
    return m_webView != NULL;
}

void FrameLoaderClientNetflix::makeRepresentation(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::forceLayout()
{
    notImplemented();
}

void FrameLoaderClientNetflix::forceLayoutForNonHTML()
{
    notImplemented();
}

void FrameLoaderClientNetflix::setCopiesOnScroll()
{
    // apparently mac specific
    notImplemented();
}

/*
  LoadErrorResetToken* FrameLoaderClientNetflix::tokenForLoadErrorReset()
  {
  notImplemented();
  return 0;
  }

  void FrameLoaderClientNetflix::resetAfterLoadError(LoadErrorResetToken*)
  {
  notImplemented();
  }

  void FrameLoaderClientNetflix::doNotResetAfterLoadError(LoadErrorResetToken*)
  {
  notImplemented();
  }
*/

void FrameLoaderClientNetflix::detachedFromParent2()
{
    notImplemented();
}

void FrameLoaderClientNetflix::detachedFromParent3()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidHandleOnloadEvents()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidCancelClientRedirect()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchWillPerformClientRedirect(const KURL&,
                                                                 double interval,
                                                                 double fireDate)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidChangeLocationWithinPage()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchWillClose()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidStartProvisionalLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidReceiveTitle(const StringWithDirection& title)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidCommitLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFinishDocumentLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFinishLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFirstLayout()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFirstVisuallyNonEmptyLayout()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchShow()
{
    notImplemented();
}

void FrameLoaderClientNetflix::cancelPolicyCheck()
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchWillSubmitForm(FramePolicyFunction function,
                                                      PassRefPtr<FormState>)
{
    // FIX: Send an event to allow for alerts and cancellation
    if (!m_frame)
        return;
    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientNetflix::dispatchDidLoadMainResource(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::revertToProvisionalState(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::postProgressStartedNotification()
{
    notImplemented();
}

void FrameLoaderClientNetflix::postProgressEstimateChangedNotification()
{
    notImplemented();
}

void FrameLoaderClientNetflix::postProgressFinishedNotification()
{
    notImplemented();
}

void FrameLoaderClientNetflix::setMainFrameDocumentReady(bool b)
{
    notImplemented();
    // this is only interesting once we provide an external API for the DOM
}

void FrameLoaderClientNetflix::willChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::didChangeTitle(DocumentLoader *l)
{
    //printf("url:%s\n", l->url().string().latin1().data());
    setTitle(l->title(), l->url());
}

void FrameLoaderClientNetflix::finishedLoading(DocumentLoader* documentLoader)
{
    if(m_firstData) {
        documentLoader->writer()->setEncoding(m_response.textEncodingName(), false);
        m_firstData = false;
    }
}

bool FrameLoaderClientNetflix::canShowMIMEType(const String& MIMEType) const
{
    notImplemented();
    return true;
}

bool FrameLoaderClientNetflix::canShowMIMETypeAsHTML(const WTF::String& MIMEType) const
{
    notImplemented();
    return true;
}

bool FrameLoaderClientNetflix::representationExistsForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return false;
}

String FrameLoaderClientNetflix::generatedMIMETypeForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return String();
}

void FrameLoaderClientNetflix::frameLoadCompleted()
{
    notImplemented();
}

void FrameLoaderClientNetflix::saveViewStateToItem(HistoryItem*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::restoreViewState()
{
    notImplemented();
}

void FrameLoaderClientNetflix::provisionalLoadStarted()
{
    notImplemented();
}

void FrameLoaderClientNetflix::didFinishLoad()
{
    notImplemented();
}

void FrameLoaderClientNetflix::prepareForDataSourceReplacement()
{
    notImplemented();
}

void FrameLoaderClientNetflix::setTitle(const StringWithDirection& title, const KURL&)
{
    notImplemented();
}

String FrameLoaderClientNetflix::userAgent(const KURL&)
{
    return String("Mozilla/5.0 (compatible; U; Netflix) Netflix/3.0.2");
}

void FrameLoaderClientNetflix::dispatchDidReceiveIcon()
{
    notImplemented();
}

void FrameLoaderClientNetflix::frameLoaderDestroyed()
{
    m_frame = 0;
    delete this;
}

bool FrameLoaderClientNetflix::canHandleRequest(const WebCore::ResourceRequest &req) const
{
    if (m_webView && !req.url().isEmpty() && req.url().protocol().length())
        return m_webView->canHandleRequest(req.url());
    return true;
}

void FrameLoaderClientNetflix::updateGlobalHistory()
{
    notImplemented();
}

void FrameLoaderClientNetflix::updateGlobalHistoryRedirectLinks()
{
    notImplemented();
}

bool FrameLoaderClientNetflix::shouldGoToHistoryItem(WebCore::HistoryItem*) const
{
    notImplemented();
    return true;
}

bool FrameLoaderClientNetflix::canCachePage() const
{
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    return true;
#else
    return false;
#endif
}

void FrameLoaderClientNetflix::setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientNetflix::committedLoad(WebCore::DocumentLoader* loader, const char* data, int length)
{
    if (!m_frame)
        return;
    loader->commitData(data, length);
}

WebCore::ResourceError FrameLoaderClientNetflix::cancelledError(const WebCore::ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::blockedError(const ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::cannotShowURLError(const WebCore::ResourceRequest& request)
{
    return ResourceError(String(), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::interruptForPolicyChangeError(const WebCore::ResourceRequest& request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowMIMEType, response.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::fileDoesNotExistError(const WebCore::ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotShowURL, response.url().string(), String());
}

WebCore::ResourceError FrameLoaderClientNetflix::interruptedForPolicyChangeError(const WebCore::ResourceRequest &request)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(), String());
}

bool FrameLoaderClientNetflix::shouldFallBack(const WebCore::ResourceError& error)
{
    notImplemented();
    return false;
}

WTF::PassRefPtr<DocumentLoader> FrameLoaderClientNetflix::createDocumentLoader(const ResourceRequest& request, const SubstituteData& substituteData)
{
    RefPtr<DocumentLoader> loader = DocumentLoader::create(request, substituteData);
    if (substituteData.isValid()) {
        loader->setDeferMainResourceDataLoad(false);
        // Use the default timeout interval for JS as the HTML tokenizer delay. This ensures
        // that long-running JavaScript will still allow setHtml() to be synchronous, while
        // still giving a reasonable timeout to prevent deadlock.
#if USE(JSC)
        double delay = JSDOMWindowBase::commonJSGlobalData()->timeoutChecker.timeoutInterval() / 1000.0f;
#elif USE(V8)
        // FIXME: Hard coded for now.
        double delay = 10000 / 1000.0f;
#endif
        m_frame->page()->setCustomHTMLTokenizerTimeDelay(delay);
    } else
        m_frame->page()->setCustomHTMLTokenizerTimeDelay(-1);
    return loader.release();
}

void FrameLoaderClientNetflix::download(ResourceHandle*, const ResourceRequest&, const ResourceRequest&, const ResourceResponse&)
{
    notImplemented();
}

void FrameLoaderClientNetflix::assignIdentifierToInitialRequest(unsigned long identifier, DocumentLoader*, const ResourceRequest&)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchWillSendRequest(DocumentLoader*, unsigned long, ResourceRequest& request, const ResourceResponse& response)
{
    const KURL & orig = request.url();
    if (m_webView) {
        KURL mod = m_webView->notifyRequest(orig);
        if(mod != orig) {
            printf("wanted to load %s, changing to %s\n", orig.string().latin1().data(), mod.string().latin1().data());
            request.setURL(mod);
        }
    }
}

bool FrameLoaderClientNetflix::shouldUseCredentialStorage(DocumentLoader*, unsigned long)
{
    notImplemented();
    return false;
}

void FrameLoaderClientNetflix::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidReceiveResponse(DocumentLoader* loader, unsigned long id, const ResourceResponse& response)
{
    notImplemented();
    m_response = response;
    m_firstData = true;
}

void FrameLoaderClientNetflix::dispatchDidReceiveContentLength(DocumentLoader* loader, unsigned long id, int length)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFinishLoading(DocumentLoader*, unsigned long)
{
    notImplemented();
}

void FrameLoaderClientNetflix::dispatchDidFailLoading(DocumentLoader*, unsigned long, const ResourceError&)
{
#if 0
      if (m_webView)
            m_webView->notifyLoadFail();
#endif
}

bool FrameLoaderClientNetflix::dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int)
{
    notImplemented();
    return false;
}

void FrameLoaderClientNetflix::dispatchDidFailProvisionalLoad(const ResourceError & error)
{
    if ( !error.isNull() && !error.isCancellation() && m_webView )
        m_webView->notify(NetflixEventLoadFail);
}

void FrameLoaderClientNetflix::dispatchDidFailLoad(const ResourceError & error)
{
    if ( !error.isNull() && !error.isCancellation() && m_webView )
        m_webView->notify(NetflixEventLoadFail);
}

Frame* FrameLoaderClientNetflix::dispatchCreatePage(const WebCore::NavigationAction&)
{
    notImplemented();
    return false;
}

void FrameLoaderClientNetflix::dispatchDecidePolicyForResponse(FramePolicyFunction function, const WebCore::ResourceResponse &response, const WebCore::ResourceRequest &request)
{
    if (!m_frame)
        return;

    notImplemented();
    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientNetflix::dispatchDecidePolicyForMIMEType(FramePolicyFunction function, const String& mimetype, const ResourceRequest& request)
{
    if (!m_frame)
        return;

    notImplemented();
    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientNetflix::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const NavigationAction&,
                                                                      const ResourceRequest& request, PassRefPtr<FormState>, const String& targetName)
{
    if (!m_frame)
        return;

    if (m_webView) {
    }

    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientNetflix::dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const NavigationAction& action,
                                                                       const ResourceRequest& request, PassRefPtr<FormState>)
{
    if (!m_frame)
        return;

    if (m_webView)
        (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientNetflix::dispatchUnableToImplementPolicy(const ResourceError & error)
{
    if ( !error.isNull() && !error.isCancellation() && error.errorCode() == WebKitErrorCannotShowURL && m_webView )
        m_webView->notify(NetflixEventLoadFail);
}

void FrameLoaderClientNetflix::startDownload(const ResourceRequest&, const WTF::String &)
{
    notImplemented();
}

PassRefPtr<Frame> FrameLoaderClientNetflix::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement,
                                                        const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    // FIXME: We should apply the right property to the frameView. (scrollbar,margins)

    FrameLoaderClientNetflix *loaderClient = new FrameLoaderClientNetflix();
    RefPtr<Frame> childFrame = Frame::create(m_frame->page(), ownerElement, loaderClient);
    loaderClient->setFrame(childFrame.get());
    loaderClient->setWebView(m_webView);

    childFrame->tree()->setName(name);

    m_frame->tree()->appendChild(childFrame);

    childFrame->init();

    // The creation of the frame may have run arbitrary JavaScript that removed it from the page already.
    if (!childFrame->tree()->parent())
        return 0;

    if (m_webView)
        m_webView->notify(NetflixEventFrameCreate, childFrame.get());

    m_frame->loader()->loadURLIntoChildFrame(url, referrer, childFrame.get());

    // The frame's onload handler may have removed it from the document.
    if (!childFrame->tree()->parent())
        return 0;

    return childFrame.release();
}

ObjectContentType FrameLoaderClientNetflix::objectContentType(const KURL& url, const String& mimeType, bool)
{
    notImplemented();
    return ObjectContentType();
}

PassRefPtr<Widget> FrameLoaderClientNetflix::createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const Vector<String>&,
                                                          const Vector<String>&, const String&, bool loadManually)
{
    notImplemented();
    return 0;
}

void FrameLoaderClientNetflix::redirectDataToPlugin(Widget* pluginWidget)
{
    notImplemented();
    return;
}

ResourceError FrameLoaderClientNetflix::pluginWillHandleLoadError(const ResourceResponse& response)
{
    notImplemented();
    return ResourceError(String(), WebKitErrorCannotLoadPlugIn, response.url().string(), String());
}

PassRefPtr<Widget> FrameLoaderClientNetflix::createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL& baseURL,
                                                                    const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    notImplemented();
    return 0;
}

String FrameLoaderClientNetflix::overrideMediaType() const
{
    notImplemented();
    return String();
}

void FrameLoaderClientNetflix::dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld*)
{
    if (m_webView)
        m_webView->notify(NetflixEventWindowClear, m_webView->m_frame.get());
}


void FrameLoaderClientNetflix::documentElementAvailable()
{
    notImplemented();
}

void FrameLoaderClientNetflix::didPerformFirstNavigation() const
{
    notImplemented();
}

void FrameLoaderClientNetflix::registerForIconNotification(bool listen)
{
    notImplemented();
}

void FrameLoaderClientNetflix::savePlatformDataToCachedFrame(CachedFrame*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::transitionToCommittedFromCachedFrame(CachedFrame*)
{
    notImplemented();
}

void FrameLoaderClientNetflix::transitionToCommittedForNewPage()
{
    ASSERT(m_frame);
    ASSERT(m_webView);

    Page* page = m_frame->page();
    ASSERT(page);

    bool isMainFrame = m_frame == page->mainFrame();

    if (isMainFrame && m_frame->view())
        m_frame->view()->setParentVisible(false);
    m_frame->setView(0);

    RefPtr<FrameView> frameView;
    if (isMainFrame) {
        WebCore::IntSize sz(m_webView->getWidth(), m_webView->getHeight());
        frameView = FrameView::create(m_frame, sz);
        const WebCore::Color color = m_webView->backgroundColor();
        frameView->setBaseBackgroundColor(color);
        if (color.alpha() == 0)
            frameView->setTransparent(true);
        frameView->setParentVisible(true);
    } else {
        frameView = FrameView::create(m_frame);
    }

    ASSERT(frameView);
    m_frame->setView(frameView);

    if (HTMLFrameOwnerElement* owner = m_frame->ownerElement())
        m_frame->view()->setScrollbarModes(owner->scrollingMode(), owner->scrollingMode());
}

void FrameLoaderClientNetflix::didDisplayInsecureContent()
{
}

void FrameLoaderClientNetflix::didRunInsecureContent(SecurityOrigin*, const WebCore::KURL &)
{
}

void
FrameLoaderClientNetflix::transferLoadingResourceFromPage(long unsigned int, WebCore::DocumentLoader*, const WebCore::ResourceRequest&, WebCore::Page*)
{
    
}

void
FrameLoaderClientNetflix::didRestoreFromPageCache()
{
    
}

void
FrameLoaderClientNetflix::didSaveToPageCache()
{
    
}

WTF::PassRefPtr<WebCore::FrameNetworkingContext>
FrameLoaderClientNetflix::createNetworkingContext()
{
    return FrameNetworkingContextNetflix::create(m_frame);
}


}

