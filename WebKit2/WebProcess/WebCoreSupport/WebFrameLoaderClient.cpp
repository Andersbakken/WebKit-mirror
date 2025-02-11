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

#include "WebFrameLoaderClient.h"

#define DISABLE_NOT_IMPLEMENTED_WARNINGS 1
#include "NotImplemented.h"

#include "AuthenticationManager.h"
#include "DataReference.h"
#include "InjectedBundleUserMessageCoders.h"
#include "PlatformCertificateInfo.h"
#include "PluginView.h"
#include "StringPairVector.h"
#include "WebContextMessages.h"
#include "WebCoreArgumentCoders.h"
#include "WebErrors.h"
#include "WebEvent.h"
#include "WebFrame.h"
#include "WebFrameNetworkingContext.h"
#include "WebNavigationDataStore.h"
#include "WebPage.h"
#include "WebPageProxyMessages.h"
#include "WebProcess.h"
#include "WebProcessProxyMessageKinds.h"
#include "WebProcessProxyMessages.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JSObject.h>
#include <WebCore/Chrome.h>
#include <WebCore/DOMWrapperWorld.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/FormState.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoadRequest.h>
#include <WebCore/FrameView.h>
#include <WebCore/HTMLAppletElement.h>
#include <WebCore/HTMLFormElement.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/MouseEvent.h>
#include <WebCore/Page.h>
#include <WebCore/PluginData.h>
#include <WebCore/ProgressTracker.h>
#include <WebCore/ResourceError.h>
#include <WebCore/UIEventWithKeyState.h>
#include <WebCore/Widget.h>
#include <WebCore/WindowFeatures.h>

using namespace WebCore;

namespace WebKit {

WebFrameLoaderClient::WebFrameLoaderClient(WebFrame* frame)
    : m_frame(frame)
    , m_hasSentResponseToPluginView(false)
    , m_frameHasCustomRepresentation(false)
{
}

WebFrameLoaderClient::~WebFrameLoaderClient()
{
}
    
void WebFrameLoaderClient::frameLoaderDestroyed()
{
    m_frame->invalidate();

    // Balances explicit ref() in WebFrame::createMainFrame and WebFrame::createSubframe.
    m_frame->deref();
}

bool WebFrameLoaderClient::hasHTMLView() const
{
    return !m_frameHasCustomRepresentation;
}

bool WebFrameLoaderClient::hasWebView() const
{
    return m_frame->page();
}

void WebFrameLoaderClient::makeRepresentation(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::forceLayout()
{
    notImplemented();
}

void WebFrameLoaderClient::forceLayoutForNonHTML()
{
    notImplemented();
}

void WebFrameLoaderClient::setCopiesOnScroll()
{
    notImplemented();
}

void WebFrameLoaderClient::detachedFromParent2()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didRemoveFrameFromHierarchy(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidRemoveFrameFromHierarchy(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));

}

void WebFrameLoaderClient::detachedFromParent3()
{
    notImplemented();
}

void WebFrameLoaderClient::assignIdentifierToInitialRequest(unsigned long identifier, DocumentLoader*, const ResourceRequest& request)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidInitiateLoadForResource(m_frame->frameID(), identifier, request));
}

void WebFrameLoaderClient::dispatchWillSendRequest(DocumentLoader*, unsigned long identifier, ResourceRequest& request, const ResourceResponse& redirectResponse)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidSendRequestForResource(m_frame->frameID(), identifier, request, redirectResponse));
}

bool WebFrameLoaderClient::shouldUseCredentialStorage(DocumentLoader*, unsigned long identifier)
{
    return true;
}

void WebFrameLoaderClient::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, unsigned long, const AuthenticationChallenge& challenge)
{
    // FIXME: Authentication is a per-resource concept, but we don't do per-resource handling in the UIProcess at the API level quite yet.
    // Once we do, we might need to make sure authentication fits with our solution.

    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    AuthenticationManager::shared().didReceiveAuthenticationChallenge(m_frame, challenge);
}

void WebFrameLoaderClient::dispatchDidCancelAuthenticationChallenge(DocumentLoader*, unsigned long identifier, const AuthenticationChallenge&)    
{
    notImplemented();
}

#if USE(PROTECTION_SPACE_AUTH_CALLBACK)
bool WebFrameLoaderClient::canAuthenticateAgainstProtectionSpace(DocumentLoader*, unsigned long, const ProtectionSpace& protectionSpace)
{
    // FIXME: Authentication is a per-resource concept, but we don't do per-resource handling in the UIProcess at the API level quite yet.
    // Once we do, we might need to make sure authentication fits with our solution.
    
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return false;
        
    bool canAuthenticate;
    if (!webPage->sendSync(Messages::WebPageProxy::CanAuthenticateAgainstProtectionSpaceInFrame(m_frame->frameID(), protectionSpace), Messages::WebPageProxy::CanAuthenticateAgainstProtectionSpaceInFrame::Reply(canAuthenticate)))
        return false;
    
    return canAuthenticate;
}
#endif

void WebFrameLoaderClient::dispatchDidReceiveResponse(DocumentLoader*, unsigned long identifier, const ResourceResponse& response)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidReceiveResponseForResource(m_frame->frameID(), identifier, response));
}

void WebFrameLoaderClient::dispatchDidReceiveContentLength(DocumentLoader*, unsigned long identifier, int lengthReceived)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidReceiveContentLengthForResource(m_frame->frameID(), identifier, lengthReceived));
}

void WebFrameLoaderClient::dispatchDidFinishLoading(DocumentLoader*, unsigned long identifier)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidFinishLoadForResource(m_frame->frameID(), identifier));
}

void WebFrameLoaderClient::dispatchDidFailLoading(DocumentLoader*, unsigned long identifier, const ResourceError& error)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::DidFailLoadForResource(m_frame->frameID(), identifier, error));
}

bool WebFrameLoaderClient::dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int length)
{
    notImplemented();
    return false;
}

void WebFrameLoaderClient::dispatchDidLoadResourceByXMLHttpRequest(unsigned long identifier, const String&)
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidHandleOnloadEvents()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didHandleOnloadEventsForFrame(webPage, m_frame);
}

void WebFrameLoaderClient::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    DocumentLoader* provisionalLoader = m_frame->coreFrame()->loader()->provisionalDocumentLoader();
    const String& url = provisionalLoader->url().string();
    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didReceiveServerRedirectForProvisionalLoadForFrame(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidReceiveServerRedirectForProvisionalLoadForFrame(m_frame->frameID(), url, InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidCancelClientRedirect()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didCancelClientRedirectForFrame(webPage, m_frame);
}

void WebFrameLoaderClient::dispatchWillPerformClientRedirect(const KURL& url, double interval, double fireDate)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().willPerformClientRedirectForFrame(webPage, m_frame, url.string(), interval, fireDate);
}

void WebFrameLoaderClient::dispatchDidChangeLocationWithinPage()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didSameDocumentNavigationForFrame(webPage, m_frame, SameDocumentNavigationAnchorNavigation, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidSameDocumentNavigationForFrame(m_frame->frameID(), SameDocumentNavigationAnchorNavigation, m_frame->coreFrame()->loader()->url().string(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidPushStateWithinPage()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didSameDocumentNavigationForFrame(webPage, m_frame, SameDocumentNavigationSessionStatePush, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidSameDocumentNavigationForFrame(m_frame->frameID(), SameDocumentNavigationSessionStatePush, m_frame->coreFrame()->loader()->url().string(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidReplaceStateWithinPage()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didSameDocumentNavigationForFrame(webPage, m_frame, SameDocumentNavigationSessionStateReplace, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidSameDocumentNavigationForFrame(m_frame->frameID(), SameDocumentNavigationSessionStateReplace, m_frame->coreFrame()->loader()->url().string(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidPopStateWithinPage()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didSameDocumentNavigationForFrame(webPage, m_frame, SameDocumentNavigationSessionStatePop, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidSameDocumentNavigationForFrame(m_frame->frameID(), SameDocumentNavigationSessionStatePop, m_frame->coreFrame()->loader()->url().string(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchWillClose()
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidReceiveIcon()
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidStartProvisionalLoad()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;
    webPage->findController().hideFindUI();
    
    DocumentLoader* provisionalLoader = m_frame->coreFrame()->loader()->provisionalDocumentLoader();
    const String& url = provisionalLoader->url().string();
    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didStartProvisionalLoadForFrame(webPage, m_frame, userData);

    bool loadingSubstituteDataForUnreachableURL = !provisionalLoader->unreachableURL().isNull();

    webPage->sandboxExtensionTracker().didStartProvisionalLoad(m_frame);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidStartProvisionalLoadForFrame(m_frame->frameID(), url, loadingSubstituteDataForUnreachableURL, InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidReceiveTitle(const String& title)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didReceiveTitleForFrame(webPage, title, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidReceiveTitleForFrame(m_frame->frameID(), title, InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidChangeIcons()
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidCommitLoad()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    const ResourceResponse& response = m_frame->coreFrame()->loader()->documentLoader()->response();
    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didCommitLoadForFrame(webPage, m_frame, userData);

    webPage->sandboxExtensionTracker().didCommitProvisionalLoad(m_frame);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidCommitLoadForFrame(m_frame->frameID(), response.mimeType(), m_frameHasCustomRepresentation, PlatformCertificateInfo(response), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidFailProvisionalLoad(const ResourceError& error)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFailProvisionalLoadWithErrorForFrame(webPage, m_frame, error, userData);

    webPage->sandboxExtensionTracker().didFailProvisionalLoad(m_frame);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFailProvisionalLoadForFrame(m_frame->frameID(), error, InjectedBundleUserMessageEncoder(userData.get())));
    
    // If we have a load listener, notify it.
    if (WebFrame::LoadListener* loadListener = m_frame->loadListener())
        loadListener->didFailLoad(m_frame, error.isCancellation());
}

void WebFrameLoaderClient::dispatchDidFailLoad(const ResourceError& error)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFailLoadWithErrorForFrame(webPage, m_frame, error, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFailLoadForFrame(m_frame->frameID(), error, InjectedBundleUserMessageEncoder(userData.get())));

    // If we have a load listener, notify it.
    if (WebFrame::LoadListener* loadListener = m_frame->loadListener())
        loadListener->didFailLoad(m_frame, error.isCancellation());
}

void WebFrameLoaderClient::dispatchDidFinishDocumentLoad()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFinishDocumentLoadForFrame(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFinishDocumentLoadForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidFinishLoad()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFinishLoadForFrame(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFinishLoadForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));

    // If we have a load listener, notify it.
    if (WebFrame::LoadListener* loadListener = m_frame->loadListener())
        loadListener->didFinishLoad(m_frame);
}

void WebFrameLoaderClient::dispatchDidFirstLayout()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFirstLayoutForFrame(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFirstLayoutForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidFirstVisuallyNonEmptyLayout()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    // Notify the bundle client.
    webPage->injectedBundleLoaderClient().didFirstVisuallyNonEmptyLayoutForFrame(webPage, m_frame, userData);

    // Notify the UIProcess.
    webPage->send(Messages::WebPageProxy::DidFirstVisuallyNonEmptyLayoutForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));
}

Frame* WebFrameLoaderClient::dispatchCreatePage(const NavigationAction& navigationAction)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return 0;

    // Just call through to the chrome client.
    Page* newPage = webPage->corePage()->chrome()->createWindow(m_frame->coreFrame(), FrameLoadRequest(m_frame->coreFrame()->document()->securityOrigin()), WindowFeatures(), navigationAction);
    if (!newPage)
        return 0;
    
    return newPage->mainFrame();
}

void WebFrameLoaderClient::dispatchShow()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->show();
}

uint32_t modifiersForNavigationAction(const NavigationAction& navigationAction)
{
    uint32_t modifiers = 0;
    if (const UIEventWithKeyState* keyStateEvent = findEventWithKeyState(const_cast<Event*>(navigationAction.event()))) {
        if (keyStateEvent->shiftKey())
            modifiers |= WebEvent::ShiftKey;
        if (keyStateEvent->ctrlKey())
            modifiers |= WebEvent::ControlKey;
        if (keyStateEvent->altKey())
            modifiers |= WebEvent::AltKey;
        if (keyStateEvent->metaKey())
            modifiers |= WebEvent::MetaKey;
    }

    return modifiers;
}

static const MouseEvent* findMouseEvent(const Event* event)
{
    for (const Event* e = event; e; e = e->underlyingEvent()) {
        if (e->isMouseEvent())
            return static_cast<const MouseEvent*>(e);
    }
    return 0;
}

int32_t mouseButtonForNavigationAction(const NavigationAction& navigationAction)
{
    const MouseEvent* mouseEvent = findMouseEvent(navigationAction.event());
    if (!mouseEvent)
        return -1;

    if (!mouseEvent->buttonDown())
        return -1;

    return mouseEvent->button();
}

void WebFrameLoaderClient::dispatchDecidePolicyForMIMEType(FramePolicyFunction function, const String& MIMEType, const ResourceRequest& request)
{
    if (m_frame->coreFrame()->loader()->documentLoader()->url().isEmpty() && request.url() == blankURL()) {
        // WebKit2 loads initial about:blank documents synchronously, without consulting the policy delegate
        ASSERT(m_frame->coreFrame()->loader()->stateMachine()->committingFirstRealLoad());
        (m_frame->coreFrame()->loader()->policyChecker()->*function)(PolicyUse);
        return;
    }
    
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    uint64_t listenerID = m_frame->setUpPolicyListener(function);
    const String& url = request.url().string(); // FIXME: Pass entire request.

    bool receivedPolicyAction;
    uint64_t policyAction;
    uint64_t downloadID;
    if (!webPage->sendSync(Messages::WebPageProxy::DecidePolicyForMIMEType(m_frame->frameID(), MIMEType, url, listenerID), Messages::WebPageProxy::DecidePolicyForMIMEType::Reply(receivedPolicyAction, policyAction, downloadID)))
        return;

    // We call this synchronously because CFNetwork can only convert a loading connection to a download from its didReceiveResponse callback.
    if (receivedPolicyAction)
        m_frame->didReceivePolicyDecision(listenerID, static_cast<PolicyAction>(policyAction), downloadID);
}

void WebFrameLoaderClient::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const NavigationAction& navigationAction, const ResourceRequest& request, PassRefPtr<FormState>, const String& frameName)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    uint64_t listenerID = m_frame->setUpPolicyListener(function);

    // FIXME: Pass more than just the navigation action type.
    // FIXME: Pass the frame name.
    const String& url = request.url().string(); // FIXME: Pass entire request.

    uint32_t navigationType = static_cast<uint32_t>(navigationAction.type());
    uint32_t modifiers = modifiersForNavigationAction(navigationAction);
    int32_t mouseButton = mouseButtonForNavigationAction(navigationAction);

    webPage->send(Messages::WebPageProxy::DecidePolicyForNewWindowAction(m_frame->frameID(), navigationType, modifiers, mouseButton, url, listenerID));
}

void WebFrameLoaderClient::dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const NavigationAction& navigationAction, const ResourceRequest& request, PassRefPtr<FormState>)
{
    if (m_frame->coreFrame()->loader()->documentLoader()->url().isEmpty() && request.url() == blankURL()) {
        // WebKit2 loads initial about:blank documents synchronously, without consulting the policy delegate
        ASSERT(m_frame->coreFrame()->loader()->stateMachine()->committingFirstRealLoad());
        (m_frame->coreFrame()->loader()->policyChecker()->*function)(PolicyUse);
        return;
    }

    // Always ignore requests with empty URLs.
    if (request.isEmpty()) {
        (m_frame->coreFrame()->loader()->policyChecker()->*function)(PolicyIgnore);
        return;
    }
    
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    uint64_t listenerID = m_frame->setUpPolicyListener(function);

    // FIXME: Pass more than just the navigation action type.
    const String& url = request.url().string(); // FIXME: Pass entire request.

    uint32_t navigationType = static_cast<uint32_t>(navigationAction.type());
    uint32_t modifiers = modifiersForNavigationAction(navigationAction);
    int32_t mouseButton = mouseButtonForNavigationAction(navigationAction);

    webPage->send(Messages::WebPageProxy::DecidePolicyForNavigationAction(m_frame->frameID(), navigationType, modifiers, mouseButton, url, listenerID));
}

void WebFrameLoaderClient::cancelPolicyCheck()
{
    m_frame->invalidatePolicyListener();
}

void WebFrameLoaderClient::dispatchUnableToImplementPolicy(const ResourceError&)
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchWillSubmitForm(FramePolicyFunction function, PassRefPtr<FormState> prpFormState)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    // FIXME: Pass more of the form state.
    RefPtr<FormState> formState = prpFormState;
    
    HTMLFormElement* form = formState->form();
    WebFrame* sourceFrame = static_cast<WebFrameLoaderClient*>(formState->sourceFrame()->loader()->client())->webFrame();    
    const Vector<std::pair<String, String> >& values = formState->textFieldValues();

    RefPtr<APIObject> userData;
    webPage->injectedBundleFormClient().willSubmitForm(webPage, form, m_frame, sourceFrame, values, userData);


    uint64_t listenerID = m_frame->setUpPolicyListener(function);
    StringPairVector valuesVector(values);

    webPage->send(Messages::WebPageProxy::WillSubmitForm(m_frame->frameID(), sourceFrame->frameID(), valuesVector, listenerID, InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::dispatchDidLoadMainResource(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::revertToProvisionalState(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::setMainDocumentError(DocumentLoader*, const ResourceError& error)
{
    if (!m_pluginView)
        return;
    
    m_pluginView->manualLoadDidFail(error);
    m_pluginView = 0;
    m_hasSentResponseToPluginView = false;
}

void WebFrameLoaderClient::willChangeEstimatedProgress()
{
    notImplemented();
}

void WebFrameLoaderClient::didChangeEstimatedProgress()
{
    notImplemented();
}

void WebFrameLoaderClient::postProgressStartedNotification()
{
    if (WebPage* webPage = m_frame->page())
        webPage->send(Messages::WebPageProxy::DidStartProgress());
}

void WebFrameLoaderClient::postProgressEstimateChangedNotification()
{
    if (WebPage* webPage = m_frame->page()) {
        double progress = webPage->corePage()->progress()->estimatedProgress();
        webPage->send(Messages::WebPageProxy::DidChangeProgress(progress));

    }
}

void WebFrameLoaderClient::postProgressFinishedNotification()
{
    if (WebPage* webPage = m_frame->page())
        webPage->send(Messages::WebPageProxy::DidFinishProgress());
}

void WebFrameLoaderClient::setMainFrameDocumentReady(bool)
{
    notImplemented();
}

void WebFrameLoaderClient::startDownload(const ResourceRequest& request)
{
    m_frame->startDownload(request);
}

void WebFrameLoaderClient::willChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::didChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::committedLoad(DocumentLoader* loader, const char* data, int length)
{
    // If we're loading a custom representation, we don't want to hand off the data to WebCore.
    if (m_frameHasCustomRepresentation)
        return;

    if (!m_pluginView)
        loader->commitData(data, length);

    // If the document is a stand-alone media document, now is the right time to cancel the WebKit load.
    // FIXME: This code should be shared across all ports. <http://webkit.org/b/48762>.
    if (m_frame->coreFrame()->document()->isMediaDocument())
        loader->cancelMainResourceLoad(pluginWillHandleLoadError(loader->response()));

    // Calling commitData did not create the plug-in view.
    if (!m_pluginView)
        return;

    if (!m_hasSentResponseToPluginView) {
        m_pluginView->manualLoadDidReceiveResponse(loader->response());
        // manualLoadDidReceiveResponse sets up a new stream to the plug-in. on a full-page plug-in, a failure in
        // setting up this stream can cause the main document load to be cancelled, setting m_pluginView
        // to null
        if (!m_pluginView)
            return;
        m_hasSentResponseToPluginView = true;
    }
    m_pluginView->manualLoadDidReceiveData(data, length);
}

void WebFrameLoaderClient::finishedLoading(DocumentLoader* loader)
{
    if (!m_pluginView) {
        committedLoad(loader, 0, 0);

        if (m_frameHasCustomRepresentation) {
            WebPage* webPage = m_frame->page();
            if (!webPage)
                return;

            RefPtr<SharedBuffer> mainResourceData = loader->mainResourceData();
            CoreIPC::DataReference dataReference(reinterpret_cast<const uint8_t*>(mainResourceData ? mainResourceData->data() : 0), mainResourceData ? mainResourceData->size() : 0);
            
            webPage->send(Messages::WebPageProxy::DidFinishLoadingDataForCustomRepresentation(dataReference));
        }

        return;
    }

    m_pluginView->manualLoadDidFinishLoading();
    m_pluginView = 0;
    m_hasSentResponseToPluginView = false;
}

void WebFrameLoaderClient::updateGlobalHistory()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    DocumentLoader* loader = m_frame->coreFrame()->loader()->documentLoader();

    WebNavigationDataStore data;
    data.url = loader->urlForHistory().string();
    data.title = loader->title();

    WebProcess::shared().connection()->send(Messages::WebContext::DidNavigateWithNavigationData(webPage->pageID(), data, m_frame->frameID()), 0);
}

void WebFrameLoaderClient::updateGlobalHistoryRedirectLinks()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    DocumentLoader* loader = m_frame->coreFrame()->loader()->documentLoader();
    ASSERT(loader->unreachableURL().isEmpty());

    // Client redirect
    if (!loader->clientRedirectSourceForHistory().isNull()) {
        WebProcess::shared().connection()->send(Messages::WebContext::DidPerformClientRedirect(webPage->pageID(),
            loader->clientRedirectSourceForHistory(), loader->clientRedirectDestinationForHistory(), m_frame->frameID()), 0);
    }

    // Server redirect
    if (!loader->serverRedirectSourceForHistory().isNull()) {
        WebProcess::shared().connection()->send(Messages::WebContext::DidPerformServerRedirect(webPage->pageID(),
            loader->serverRedirectSourceForHistory(), loader->serverRedirectDestinationForHistory(), m_frame->frameID()), 0);
    }
}

bool WebFrameLoaderClient::shouldGoToHistoryItem(HistoryItem*) const
{
    notImplemented();
    return true;
}

void WebFrameLoaderClient::dispatchDidAddBackForwardItem(HistoryItem*) const
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidRemoveBackForwardItem(HistoryItem*) const
{
    notImplemented();
}

void WebFrameLoaderClient::dispatchDidChangeBackForwardIndex() const
{
    notImplemented();
}

void WebFrameLoaderClient::didDisplayInsecureContent()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    webPage->injectedBundleLoaderClient().didDisplayInsecureContentForFrame(webPage, m_frame, userData);

    webPage->send(Messages::WebPageProxy::DidDisplayInsecureContentForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));
}

void WebFrameLoaderClient::didRunInsecureContent(SecurityOrigin*)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    RefPtr<APIObject> userData;

    webPage->injectedBundleLoaderClient().didRunInsecureContentForFrame(webPage, m_frame, userData);

    webPage->send(Messages::WebPageProxy::DidRunInsecureContentForFrame(m_frame->frameID(), InjectedBundleUserMessageEncoder(userData.get())));
}

ResourceError WebFrameLoaderClient::cancelledError(const ResourceRequest& request)
{
    return WebKit::cancelledError(request);
}

ResourceError WebFrameLoaderClient::blockedError(const ResourceRequest& request)
{
    return WebKit::blockedError(request);
}

ResourceError WebFrameLoaderClient::cannotShowURLError(const ResourceRequest& request)
{
    return WebKit::cannotShowURLError(request);
}

ResourceError WebFrameLoaderClient::interruptForPolicyChangeError(const ResourceRequest& request)
{
    return WebKit::interruptForPolicyChangeError(request);
}

ResourceError WebFrameLoaderClient::cannotShowMIMETypeError(const ResourceResponse& response)
{
    return WebKit::cannotShowMIMETypeError(response);
}

ResourceError WebFrameLoaderClient::fileDoesNotExistError(const ResourceResponse& response)
{
    return WebKit::fileDoesNotExistError(response);
}

ResourceError WebFrameLoaderClient::pluginWillHandleLoadError(const ResourceResponse& response)
{
    return WebKit::pluginWillHandleLoadError(response);
}

bool WebFrameLoaderClient::shouldFallBack(const ResourceError& error)
{
    DEFINE_STATIC_LOCAL(const ResourceError, cancelledError, (this->cancelledError(ResourceRequest())));
    DEFINE_STATIC_LOCAL(const ResourceError, pluginWillHandleLoadError, (this->pluginWillHandleLoadError(ResourceResponse())));

    if (error.errorCode() == cancelledError.errorCode() && error.domain() == cancelledError.domain())
        return false;

    if (error.errorCode() == pluginWillHandleLoadError.errorCode() && error.domain() == pluginWillHandleLoadError.domain())
        return false;

    return true;
}

bool WebFrameLoaderClient::canHandleRequest(const ResourceRequest&) const
{
    notImplemented();
    return true;
}

bool WebFrameLoaderClient::canShowMIMEType(const String& MIMEType) const
{
    notImplemented();
    return true;
}

bool WebFrameLoaderClient::canShowMIMETypeAsHTML(const String& MIMEType) const
{
    return true;
}

bool WebFrameLoaderClient::representationExistsForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return false;
}

String WebFrameLoaderClient::generatedMIMETypeForURLScheme(const String& URLScheme) const
{
    notImplemented();
    return String();
}

void WebFrameLoaderClient::frameLoadCompleted()
{
    notImplemented();
}

void WebFrameLoaderClient::saveViewStateToItem(HistoryItem*)
{
    notImplemented();
}

void WebFrameLoaderClient::restoreViewState()
{
    notImplemented();
}

void WebFrameLoaderClient::provisionalLoadStarted()
{
    notImplemented();
}

void WebFrameLoaderClient::didFinishLoad()
{
    // If we have a load listener, notify it.
    if (WebFrame::LoadListener* loadListener = m_frame->loadListener())
        loadListener->didFinishLoad(m_frame);
}

void WebFrameLoaderClient::prepareForDataSourceReplacement()
{
    notImplemented();
}

PassRefPtr<DocumentLoader> WebFrameLoaderClient::createDocumentLoader(const ResourceRequest& request, const SubstituteData& data)
{
    return DocumentLoader::create(request, data);
}

void WebFrameLoaderClient::setTitle(const String& title, const KURL& url)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    WebProcess::shared().connection()->send(Messages::WebContext::DidUpdateHistoryTitle(webPage->pageID(),
        title, url.string(), m_frame->frameID()), 0);
}

String WebFrameLoaderClient::userAgent(const KURL&)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return String();

    return webPage->userAgent();
}

void WebFrameLoaderClient::savePlatformDataToCachedFrame(CachedFrame*)
{
}

void WebFrameLoaderClient::transitionToCommittedFromCachedFrame(CachedFrame*)
{
}

void WebFrameLoaderClient::transitionToCommittedForNewPage()
{
    WebPage* webPage = m_frame->page();
    Color backgroundColor = webPage->drawsTransparentBackground() ? Color::transparent : Color::white;

    bool isMainFrame = webPage->mainFrame() == m_frame;

#if ENABLE(TILED_BACKING_STORE)
    IntSize currentVisibleContentSize = m_frame->coreFrame()->view() ? m_frame->coreFrame()->view()->actualVisibleContentRect().size() : IntSize();
    m_frame->coreFrame()->createView(webPage->size(), backgroundColor, false, webPage->resizesToContentsLayoutSize(), isMainFrame && webPage->resizesToContentsEnabled());

    if (isMainFrame && webPage->resizesToContentsEnabled()) {
        m_frame->coreFrame()->view()->setDelegatesScrolling(true);
        m_frame->coreFrame()->view()->setPaintsEntireContents(true);
    }

    // The HistoryController will update the scroll position later if needed.
    m_frame->coreFrame()->view()->setActualVisibleContentRect(IntRect(IntPoint::zero(), currentVisibleContentSize));
#else
    const String& mimeType = m_frame->coreFrame()->loader()->documentLoader()->response().mimeType();
    m_frameHasCustomRepresentation = isMainFrame && WebProcess::shared().shouldUseCustomRepresentationForMIMEType(mimeType);

    m_frame->coreFrame()->createView(webPage->size(), backgroundColor, false, IntSize(), false);
#endif

    m_frame->coreFrame()->view()->setTransparent(!webPage->drawsBackground());
}

void WebFrameLoaderClient::didSaveToPageCache()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    if (m_frame->isMainFrame())
        return;

    webPage->send(Messages::WebPageProxy::DidSaveFrameToPageCache(m_frame->frameID()));
}

void WebFrameLoaderClient::didRestoreFromPageCache()
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    if (m_frame->isMainFrame())
        return;

    WebFrame* parentFrame = static_cast<WebFrameLoaderClient*>(m_frame->coreFrame()->tree()->parent()->loader()->client())->webFrame();
    webPage->send(Messages::WebPageProxy::DidRestoreFrameFromPageCache(m_frame->frameID(), parentFrame->frameID()));
}

void WebFrameLoaderClient::dispatchDidBecomeFrameset(bool value)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->send(Messages::WebPageProxy::FrameDidBecomeFrameSet(m_frame->frameID(), value));
}

bool WebFrameLoaderClient::canCachePage() const
{
    // We cannot cache frames that have custom representations because they are
    // rendered in the UIProcess. 
    return !m_frameHasCustomRepresentation;
}

void WebFrameLoaderClient::download(ResourceHandle* handle, const ResourceRequest& request, const ResourceRequest& initialRequest, const ResourceResponse& response)
{
    m_frame->convertHandleToDownload(handle, request, initialRequest, response);
}

PassRefPtr<Frame> WebFrameLoaderClient::createFrame(const KURL& url, const String& name, HTMLFrameOwnerElement* ownerElement,
                                                    const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    WebPage* webPage = m_frame->page();

    RefPtr<WebFrame> subframe = WebFrame::createSubframe(webPage, name, ownerElement);

    Frame* coreSubframe = subframe->coreFrame();

     // The creation of the frame may have run arbitrary JavaScript that removed it from the page already.
    m_frame->coreFrame()->loader()->loadURLIntoChildFrame(url, referrer, coreSubframe);

    // The frame's onload handler may have removed it from the document.
    if (!coreSubframe->tree()->parent())
        return 0;

    return coreSubframe;
}

void WebFrameLoaderClient::didTransferChildFrameToNewDocument(Page*)
{
    notImplemented();
}

void WebFrameLoaderClient::transferLoadingResourceFromPage(unsigned long, DocumentLoader*, const ResourceRequest&, Page*)
{
    notImplemented();
}

PassRefPtr<Widget> WebFrameLoaderClient::createPlugin(const IntSize&, HTMLPlugInElement* pluginElement, const KURL& url, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually)
{
    ASSERT(paramNames.size() == paramValues.size());
    
    WebPage* webPage = m_frame->page();
    ASSERT(webPage);
    
    Plugin::Parameters parameters;
    parameters.url = url;
    parameters.names = paramNames;
    parameters.values = paramValues;
    parameters.mimeType = mimeType;
    parameters.loadManually = loadManually;

    // <rdar://problem/8440903>: AppleConnect has a bug where it does not
    // understand the parameter names specified in the <object> element that
    // embeds its plug-in. This hack works around the issue by converting the
    // parameter names to lowercase before passing them to the plug-in.
    // FIXME: This workaround should be dependent on site-specific quirks being
    // enabled. This requires adding this setting to WebKit2's WebPreferences
    // implementation. See <https://bugs.webkit.org/show_bug.cgi?id=46076>.
    if (equalIgnoringCase(mimeType, "application/x-snkp")) {
        for (size_t i = 0; i < paramNames.size(); ++i)
            parameters.names[i] = paramNames[i].lower();
    }

    RefPtr<Plugin> plugin = webPage->createPlugin(parameters);
    if (!plugin)
        return 0;
    
    return PluginView::create(pluginElement, plugin.release(), parameters);
}

void WebFrameLoaderClient::redirectDataToPlugin(Widget* pluginWidget)
{
    ASSERT(!m_pluginView);
    ASSERT(pluginWidget);
    
    m_pluginView = static_cast<PluginView*>(pluginWidget);
}

PassRefPtr<Widget> WebFrameLoaderClient::createJavaAppletWidget(const IntSize& pluginSize, HTMLAppletElement* appletElement, const KURL& baseURL, const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    return createPlugin(pluginSize, appletElement, KURL(), paramNames, paramValues, "application/x-java-applet", false);
}

ObjectContentType WebFrameLoaderClient::objectContentType(const KURL& url, const String& mimeTypeIn)
{
    // FIXME: This should be merged with WebCore::FrameLoader::defaultObjectContentType when the plugin code
    // is consolidated.

    String mimeType = mimeTypeIn;
    if (mimeType.isEmpty())
        mimeType = MIMETypeRegistry::getMIMETypeForExtension(url.path().substring(url.path().reverseFind('.') + 1));

    if (mimeType.isEmpty())
        return ObjectContentFrame;

    if (MIMETypeRegistry::isSupportedImageMIMEType(mimeType))
        return WebCore::ObjectContentImage;

    if (WebPage* webPage = m_frame->page()) {
        if (PluginData* pluginData = webPage->corePage()->pluginData()) {
            if (pluginData->supportsMimeType(mimeType))
                return ObjectContentNetscapePlugin;
        }
    }

    if (MIMETypeRegistry::isSupportedNonImageMIMEType(mimeType))
        return ObjectContentFrame;

    return ObjectContentNone;
}

String WebFrameLoaderClient::overrideMediaType() const
{
    notImplemented();
    return String();
}

void WebFrameLoaderClient::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld* world)
{
    WebPage* webPage = m_frame->page();
    if (!webPage)
        return;

    webPage->injectedBundleLoaderClient().didClearWindowObjectForFrame(webPage, m_frame, world);
}

void WebFrameLoaderClient::documentElementAvailable()
{
    notImplemented();
}

void WebFrameLoaderClient::didPerformFirstNavigation() const
{
    notImplemented();
}

void WebFrameLoaderClient::registerForIconNotification(bool listen)
{
    notImplemented();
}

#if PLATFORM(MAC)
#if ENABLE(MAC_JAVA_BRIDGE)
jobject WebFrameLoaderClient::javaApplet(NSView*) { return 0; }
#endif
NSCachedURLResponse* WebFrameLoaderClient::willCacheResponse(DocumentLoader*, unsigned long identifier, NSCachedURLResponse* response) const
{
    return response;
}

#endif
#if USE(CFNETWORK)
bool WebFrameLoaderClient::shouldCacheResponse(DocumentLoader*, unsigned long identifier, const ResourceResponse&, const unsigned char* data, unsigned long long length)
{
    return true;
}

#endif

bool WebFrameLoaderClient::shouldUsePluginDocument(const String& /*mimeType*/) const
{
    notImplemented();
    return false;
}

PassRefPtr<FrameNetworkingContext> WebFrameLoaderClient::createNetworkingContext()
{
    return WebFrameNetworkingContext::create(m_frame->coreFrame());
}

} // namespace WebKit
