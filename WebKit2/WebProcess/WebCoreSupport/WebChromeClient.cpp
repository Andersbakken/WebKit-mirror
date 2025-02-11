/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "WebChromeClient.h"

#define DISABLE_NOT_IMPLEMENTED_WARNINGS 1
#include "NotImplemented.h"

#include "DrawingArea.h"
#include "InjectedBundleUserMessageCoders.h"
#include "WebContextMenu.h"
#include "WebCoreArgumentCoders.h"
#include "WebFrame.h"
#include "WebFrameLoaderClient.h"
#include "WebOpenPanelParameters.h"
#include "WebOpenPanelResultListener.h"
#include "WebPage.h"
#include "WebPageCreationParameters.h"
#include "WebPageProxyMessages.h"
#include "WebPopupMenu.h"
#include "WebPreferencesStore.h"
#include "WebProcess.h"
#include "WebProcessProxyMessageKinds.h"
#include "WebSearchPopupMenu.h"
#include <WebCore/DatabaseTracker.h>
#include <WebCore/FileChooser.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/HTMLPlugInImageElement.h>
#include <WebCore/Page.h>
#include <WebCore/SecurityOrigin.h>

using namespace WebCore;
using namespace HTMLNames;

namespace WebKit {

void WebChromeClient::chromeDestroyed()
{
    delete this;
}

void WebChromeClient::setWindowRect(const FloatRect& windowFrame)
{
    m_page->send(Messages::WebPageProxy::SetWindowFrame(windowFrame));
}

FloatRect WebChromeClient::windowRect()
{
    FloatRect newWindowFrame;

    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::GetWindowFrame(), Messages::WebPageProxy::GetWindowFrame::Reply(newWindowFrame), m_page->pageID()))
        return FloatRect();

    return newWindowFrame;
}

FloatRect WebChromeClient::pageRect()
{
    return FloatRect(FloatPoint(), m_page->size());
}

float WebChromeClient::scaleFactor()
{
    notImplemented();
    return 1.0;
}

void WebChromeClient::focus()
{
    notImplemented();
}

void WebChromeClient::unfocus()
{
    notImplemented();
}

bool WebChromeClient::canTakeFocus(FocusDirection)
{
    notImplemented();
    return true;
}

void WebChromeClient::takeFocus(FocusDirection direction)
{
    m_page->send(Messages::WebPageProxy::TakeFocus(direction == FocusDirectionForward ? true : false));
}

void WebChromeClient::focusedNodeChanged(Node*)
{
    notImplemented();
}

void WebChromeClient::focusedFrameChanged(Frame* frame)
{
    WebFrame* webFrame = frame ? static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame() : 0;

    WebProcess::shared().connection()->send(Messages::WebPageProxy::FocusedFrameChanged(webFrame ? webFrame->frameID() : 0), m_page->pageID());
}

Page* WebChromeClient::createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures& windowFeatures, const NavigationAction& navigationAction)
{
    uint32_t modifiers = modifiersForNavigationAction(navigationAction);
    int32_t mouseButton = mouseButtonForNavigationAction(navigationAction);

    uint64_t newPageID = 0;
    WebPageCreationParameters parameters;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::CreateNewPage(windowFeatures, modifiers, mouseButton), Messages::WebPageProxy::CreateNewPage::Reply(newPageID, parameters), m_page->pageID()))
        return 0;

    if (!newPageID)
        return 0;

    WebProcess::shared().createWebPage(newPageID, parameters);
    return WebProcess::shared().webPage(newPageID)->corePage();
}

void WebChromeClient::show()
{
    m_page->show();
}

bool WebChromeClient::canRunModal()
{
    notImplemented();
    return false;
}

void WebChromeClient::runModal()
{
    notImplemented();
}

void WebChromeClient::setToolbarsVisible(bool toolbarsAreVisible)
{
    m_page->send(Messages::WebPageProxy::SetToolbarsAreVisible(toolbarsAreVisible));
}

bool WebChromeClient::toolbarsVisible()
{
    bool toolbarsAreVisible = true;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::GetToolbarsAreVisible(), Messages::WebPageProxy::GetToolbarsAreVisible::Reply(toolbarsAreVisible), m_page->pageID()))
        return true;

    return toolbarsAreVisible;
}

void WebChromeClient::setStatusbarVisible(bool statusBarIsVisible)
{
    m_page->send(Messages::WebPageProxy::SetStatusBarIsVisible(statusBarIsVisible));
}

bool WebChromeClient::statusbarVisible()
{
    bool statusBarIsVisible = true;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::GetStatusBarIsVisible(), Messages::WebPageProxy::GetStatusBarIsVisible::Reply(statusBarIsVisible), m_page->pageID()))
        return true;

    return statusBarIsVisible;
}

void WebChromeClient::setScrollbarsVisible(bool)
{
    notImplemented();
}

bool WebChromeClient::scrollbarsVisible()
{
    notImplemented();
    return true;
}

void WebChromeClient::setMenubarVisible(bool menuBarVisible)
{
    m_page->send(Messages::WebPageProxy::SetMenuBarIsVisible(menuBarVisible));
}

bool WebChromeClient::menubarVisible()
{
    bool menuBarIsVisible = true;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::GetMenuBarIsVisible(), Messages::WebPageProxy::GetMenuBarIsVisible::Reply(menuBarIsVisible), m_page->pageID()))
        return true;

    return menuBarIsVisible;
}

void WebChromeClient::setResizable(bool resizable)
{
    m_page->send(Messages::WebPageProxy::SetIsResizable(resizable));
}

void WebChromeClient::addMessageToConsole(MessageSource, MessageType, MessageLevel, const String& message, unsigned int lineNumber, const String& sourceID)
{
    // Notify the bundle client.
    m_page->injectedBundleUIClient().willAddMessageToConsole(m_page, message, lineNumber);

    notImplemented();
}

bool WebChromeClient::canRunBeforeUnloadConfirmPanel()
{
    bool canRun = false;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::CanRunBeforeUnloadConfirmPanel(), Messages::WebPageProxy::CanRunBeforeUnloadConfirmPanel::Reply(canRun), m_page->pageID()))
        return false;

    return canRun;
}

bool WebChromeClient::runBeforeUnloadConfirmPanel(const String& message, Frame* frame)
{
    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();

    bool shouldClose = false;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::RunBeforeUnloadConfirmPanel(message, webFrame->frameID()), Messages::WebPageProxy::RunBeforeUnloadConfirmPanel::Reply(shouldClose), m_page->pageID()))
        return false;

    return shouldClose;
}

void WebChromeClient::closeWindowSoon()
{
    // FIXME: This code assumes that the client will respond to a close page
    // message by actually closing the page. Safari does this, but there is
    // no guarantee that other applications will, which will leave this page
    // half detached. This approach is an inherent limitation making parts of
    // a close execute synchronously as part of window.close, but other parts
    // later on.

    m_page->corePage()->setGroupName(String());

    if (WebFrame* frame = m_page->mainFrame()) {
        if (Frame* coreFrame = frame->coreFrame())
            coreFrame->loader()->stopForUserCancel();
    }

    m_page->sendClose();
}

void WebChromeClient::runJavaScriptAlert(Frame* frame, const String& alertText)
{
    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();

    // Notify the bundle client.
    m_page->injectedBundleUIClient().willRunJavaScriptAlert(m_page, alertText, webFrame);

    WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::RunJavaScriptAlert(webFrame->frameID(), alertText), Messages::WebPageProxy::RunJavaScriptAlert::Reply(), m_page->pageID());
}

bool WebChromeClient::runJavaScriptConfirm(Frame* frame, const String& message)
{
    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();

    // Notify the bundle client.
    m_page->injectedBundleUIClient().willRunJavaScriptConfirm(m_page, message, webFrame);

    bool result = false;
    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::RunJavaScriptConfirm(webFrame->frameID(), message), Messages::WebPageProxy::RunJavaScriptConfirm::Reply(result), m_page->pageID()))
        return false;

    return result;
}

bool WebChromeClient::runJavaScriptPrompt(Frame* frame, const String& message, const String& defaultValue, String& result)
{
    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();

    // Notify the bundle client.
    m_page->injectedBundleUIClient().willRunJavaScriptPrompt(m_page, message, defaultValue, webFrame);

    if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::RunJavaScriptPrompt(webFrame->frameID(), message, defaultValue), Messages::WebPageProxy::RunJavaScriptPrompt::Reply(result), m_page->pageID()))
        return false;

    return !result.isNull();
}

void WebChromeClient::setStatusbarText(const String& statusbarText)
{
    // Notify the bundle client.
    m_page->injectedBundleUIClient().willSetStatusbarText(m_page, statusbarText);

    m_page->send(Messages::WebPageProxy::SetStatusText(statusbarText));
}

bool WebChromeClient::shouldInterruptJavaScript()
{
    notImplemented();
    return false;
}

bool WebChromeClient::tabsToLinks() const
{
    return m_page->tabsToLinks();
}

IntRect WebChromeClient::windowResizerRect() const
{
    return m_page->windowResizerRect();
}

void WebChromeClient::invalidateWindow(const IntRect&, bool)
{
    // Do nothing here, there's no concept of invalidating the window in the web process.
}

void WebChromeClient::invalidateContentsAndWindow(const IntRect& rect, bool)
{
    m_page->drawingArea()->setNeedsDisplay(rect);
}

void WebChromeClient::invalidateContentsForSlowScroll(const IntRect& rect, bool)
{
    m_page->pageDidScroll();
    m_page->drawingArea()->setNeedsDisplay(rect);
}

void WebChromeClient::scroll(const IntSize& scrollDelta, const IntRect& rectToScroll, const IntRect& clipRect)
{
    m_page->pageDidScroll();
    m_page->drawingArea()->scroll(scrollDelta, rectToScroll, clipRect);
}

#if ENABLE(TILED_BACKING_STORE)
void WebChromeClient::delegatedScrollRequested(const IntSize& scrollDelta)
{
    m_page->pageDidRequestScroll(scrollDelta);
}
#endif

IntPoint WebChromeClient::screenToWindow(const IntPoint&) const
{
    notImplemented();
    return IntPoint();
}

IntRect WebChromeClient::windowToScreen(const IntRect&) const
{
    notImplemented();
    return IntRect();
}

PlatformPageClient WebChromeClient::platformPageClient() const
{
    notImplemented();
    return 0;
}

void WebChromeClient::contentsSizeChanged(Frame* frame, const IntSize& size) const
{
#if PLATFORM(QT)
#if ENABLE(TILED_BACKING_STORE)
    if (frame->page()->mainFrame() == frame)
        m_page->resizeToContentsIfNeeded();
#endif

    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();

    if (!m_page->mainFrame() || m_page->mainFrame() != webFrame)
        return;

    m_page->send(Messages::WebPageProxy::DidChangeContentsSize(size));
#endif
}

void WebChromeClient::scrollRectIntoView(const IntRect&, const ScrollView*) const
{
    notImplemented();
}

bool WebChromeClient::shouldMissingPluginMessageBeButton() const
{
    // FIXME: <rdar://problem/8794397> We should only return true when there is a 
    // missingPluginButtonClicked callback defined on the Page UI client.
    return true;
}
    
void WebChromeClient::missingPluginButtonClicked(Element* element) const
{
    ASSERT(element->hasTagName(objectTag) || element->hasTagName(embedTag));

    HTMLPlugInImageElement* pluginElement = static_cast<HTMLPlugInImageElement*>(element);

    m_page->send(Messages::WebPageProxy::MissingPluginButtonClicked(pluginElement->serviceType(), pluginElement->url()));
}

void WebChromeClient::scrollbarsModeDidChange() const
{
    notImplemented();
}

void WebChromeClient::mouseDidMoveOverElement(const HitTestResult& hitTestResult, unsigned modifierFlags)
{
    RefPtr<APIObject> userData;

    // Notify the bundle client.
    m_page->injectedBundleUIClient().mouseDidMoveOverElement(m_page, hitTestResult, static_cast<WebEvent::Modifiers>(modifierFlags), userData);

    // Notify the UIProcess.
    m_page->send(Messages::WebPageProxy::MouseDidMoveOverElement(modifierFlags, InjectedBundleUserMessageEncoder(userData.get())));
}

void WebChromeClient::setToolTip(const String& toolTip, TextDirection)
{
    // Only send a tool tip to the WebProcess if it has changed since the last time this function was called.

    if (toolTip == m_cachedToolTip)
        return;
    m_cachedToolTip = toolTip;

    m_page->send(Messages::WebPageProxy::SetToolTip(m_cachedToolTip));
}

void WebChromeClient::print(Frame*)
{
    notImplemented();
}

#if ENABLE(DATABASE)
void WebChromeClient::exceededDatabaseQuota(Frame* frame, const String& databaseName)
{
    WebFrame* webFrame = static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame();
    SecurityOrigin* origin = frame->document()->securityOrigin();

    DatabaseDetails details = DatabaseTracker::tracker().detailsForNameAndOrigin(databaseName, origin);
    uint64_t currentQuota = DatabaseTracker::tracker().quotaForOrigin(origin);
    uint64_t newQuota = 0;
    WebProcess::shared().connection()->sendSync(
        Messages::WebPageProxy::ExceededDatabaseQuota(webFrame->frameID(), origin->databaseIdentifier(), databaseName, details.displayName(), currentQuota, details.currentUsage(), details.expectedUsage()),
        Messages::WebPageProxy::ExceededDatabaseQuota::Reply(newQuota), m_page->pageID());

    DatabaseTracker::tracker().setQuota(origin, newQuota);
}
#endif


#if ENABLE(OFFLINE_WEB_APPLICATIONS)
void WebChromeClient::reachedMaxAppCacheSize(int64_t)
{
    notImplemented();
}

void WebChromeClient::reachedApplicationCacheOriginQuota(SecurityOrigin*)
{
    notImplemented();
}
#endif

#if ENABLE(DASHBOARD_SUPPORT)
void WebChromeClient::dashboardRegionsChanged()
{
    notImplemented();
}
#endif

void WebChromeClient::populateVisitedLinks()
{
}

FloatRect WebChromeClient::customHighlightRect(Node*, const AtomicString& type, const FloatRect& lineRect)
{
    notImplemented();
    return FloatRect();
}

void WebChromeClient::paintCustomHighlight(Node*, const AtomicString& type, const FloatRect& boxRect, const FloatRect& lineRect,
                                  bool behindText, bool entireLine)
{
    notImplemented();
}

bool WebChromeClient::shouldReplaceWithGeneratedFileForUpload(const String& path, String& generatedFilename)
{
    notImplemented();
    return false;
}

String WebChromeClient::generateReplacementFile(const String& path)
{
    notImplemented();
    return String();
}

bool WebChromeClient::paintCustomScrollbar(GraphicsContext*, const FloatRect&, ScrollbarControlSize, 
                                           ScrollbarControlState, ScrollbarPart pressedPart, bool vertical,
                                           float value, float proportion, ScrollbarControlPartMask)
{
    notImplemented();
    return false;
}

bool WebChromeClient::paintCustomScrollCorner(GraphicsContext*, const FloatRect&)
{
    notImplemented();
    return false;
}

void WebChromeClient::requestGeolocationPermissionForFrame(Frame*, Geolocation*)
{
    notImplemented();
}

void WebChromeClient::cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*)
{
    notImplemented();
}

void WebChromeClient::runOpenPanel(Frame* frame, PassRefPtr<FileChooser> prpFileChooser)
{
    if (m_page->activeOpenPanelResultListener())
        return;

    RefPtr<FileChooser> fileChooser = prpFileChooser;

    m_page->setActiveOpenPanelResultListener(WebOpenPanelResultListener::create(m_page, fileChooser.get()));
    
    WebOpenPanelParameters::Data parameters;
    parameters.allowMultipleFiles = fileChooser->allowsMultipleFiles();
#if ENABLE(DIRECTORY_UPLOAD)
    parameters.allowsDirectoryUpload = fileChooser->allowsDirectoryUpload();
#else
    parameters.allowsDirectoryUpload = false;
#endif
    parameters.acceptTypes = fileChooser->acceptTypes();
    parameters.filenames = fileChooser->filenames();

    m_page->send(Messages::WebPageProxy::RunOpenPanel(static_cast<WebFrameLoaderClient*>(frame->loader()->client())->webFrame()->frameID(), parameters));
}

void WebChromeClient::chooseIconForFiles(const Vector<String>&, FileChooser*)
{
    notImplemented();
}

void WebChromeClient::setCursor(const WebCore::Cursor& cursor)
{
#if USE(LAZY_NATIVE_CURSOR)
    m_page->send(Messages::WebPageProxy::SetCursor(cursor));
#endif
}

void WebChromeClient::formStateDidChange(const Node*)
{
    notImplemented();
}

void WebChromeClient::formDidFocus(const Node*)
{ 
    notImplemented();
}

void WebChromeClient::formDidBlur(const Node*)
{
    notImplemented();
}

bool WebChromeClient::selectItemWritingDirectionIsNatural()
{
    return true;
}

PassRefPtr<WebCore::PopupMenu> WebChromeClient::createPopupMenu(WebCore::PopupMenuClient* client) const
{
    return WebPopupMenu::create(m_page, client);
}

PassRefPtr<WebCore::SearchPopupMenu> WebChromeClient::createSearchPopupMenu(WebCore::PopupMenuClient* client) const
{
    return WebSearchPopupMenu::create(m_page, client);
}

#if ENABLE(CONTEXT_MENUS)
void WebChromeClient::showContextMenu()
{
    m_page->contextMenu()->show();
}
#endif

PassOwnPtr<HTMLParserQuirks> WebChromeClient::createHTMLParserQuirks()
{
    notImplemented();
    return 0;
}

#if USE(ACCELERATED_COMPOSITING)
void WebChromeClient::attachRootGraphicsLayer(Frame*, GraphicsLayer* layer)
{
    if (layer)
        m_page->enterAcceleratedCompositingMode(layer);
    else
        m_page->exitAcceleratedCompositingMode();
}

void WebChromeClient::setNeedsOneShotDrawingSynchronization()
{
    notImplemented();
}

void WebChromeClient::scheduleCompositingLayerSync()
{
    if (m_page->drawingArea())
        m_page->drawingArea()->scheduleCompositingLayerSync();
}

#endif

#if ENABLE(NOTIFICATIONS)
WebCore::NotificationPresenter* WebChromeClient::notificationPresenter() const
{
    return 0;
}
#endif

#if ENABLE(TOUCH_EVENTS)
void WebChromeClient::needTouchEvents(bool)
{
}
#endif

#if PLATFORM(WIN)
void WebChromeClient::setLastSetCursorToCurrentCursor()
{
}
#endif

void WebChromeClient::dispatchViewportDataDidChange(const ViewportArguments& args) const
{
    m_page->send(Messages::WebPageProxy::DidChangeViewportData(args));
}

} // namespace WebKit
