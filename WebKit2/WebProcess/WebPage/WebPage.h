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

#ifndef WebPage_h
#define WebPage_h

#include "APIObject.h"
#include "DrawingArea.h"
#include "FindController.h"
#include "ImageOptions.h"
#include "InjectedBundlePageContextMenuClient.h"
#include "InjectedBundlePageEditorClient.h"
#include "InjectedBundlePageFormClient.h"
#include "InjectedBundlePageLoaderClient.h"
#include "InjectedBundlePageUIClient.h"
#include "MessageSender.h"
#include "Plugin.h"
#include "SandboxExtension.h"
#include "WebEditCommand.h"
#include <WebCore/Editor.h>
#include <WebCore/FrameLoaderTypes.h>
#include <WebCore/IntRect.h>
#include <wtf/HashMap.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

#if ENABLE(TOUCH_EVENTS)
#include <WebCore/PlatformTouchEvent.h>
#endif

namespace CoreIPC {
    class ArgumentDecoder;
    class Connection;
    class MessageID;
}

namespace WebCore {
    class GraphicsContext;
    class KeyboardEvent;
    class Page;
    class ResourceRequest;
    class SharedBuffer;
}

namespace WebKit {

class DrawingArea;
class InjectedBundleBackForwardList;
class PageOverlay;
class PluginView;
class WebContextMenu;
class WebContextMenuItemData;
class WebEvent;
class WebFrame;
class WebImage;
class WebInspector;
class WebKeyboardEvent;
class WebMouseEvent;
class WebOpenPanelResultListener;
class WebPageGroupProxy;
class WebPopupMenu;
class WebWheelEvent;

struct WebPageCreationParameters;
struct WebPreferencesStore;

#if ENABLE(TOUCH_EVENTS)
class WebTouchEvent;
#endif

class WebPage : public APIObject, public CoreIPC::MessageSender<WebPage> {
public:
    static const Type APIType = TypeBundlePage;

    static PassRefPtr<WebPage> create(uint64_t pageID, const WebPageCreationParameters&);
    virtual ~WebPage();

    // Used by MessageSender.
    CoreIPC::Connection* connection() const;
    uint64_t destinationID() const { return pageID(); }

    void close();

    WebCore::Page* corePage() const { return m_page.get(); }
    uint64_t pageID() const { return m_pageID; }

    void setSize(const WebCore::IntSize&);
    const WebCore::IntSize& size() const { return m_viewSize; }

    InjectedBundleBackForwardList* backForwardList();
    DrawingArea* drawingArea() const { return m_drawingArea.get(); }

    WebPageGroupProxy* pageGroup() const { return m_pageGroup.get(); }

#if ENABLE(INSPECTOR)
    WebInspector* inspector();
#endif

    // -- Called by the DrawingArea.
    // FIXME: We could genericize these into a DrawingArea client interface. Would that be beneficial?
    void drawRect(WebCore::GraphicsContext&, const WebCore::IntRect&);
    void layoutIfNeeded();

    // -- Called from WebCore clients.
#if !PLATFORM(MAC)
    bool handleEditingKeyboardEvent(WebCore::KeyboardEvent*);
#endif
    void show();
    String userAgent() const { return m_userAgent; }
    WebCore::IntRect windowResizerRect() const;
    bool tabsToLinks() const { return m_tabToLinks; }

    WebEditCommand* webEditCommand(uint64_t);
    void addWebEditCommand(uint64_t, WebEditCommand*);
    void removeWebEditCommand(uint64_t);
    bool isInRedo() const { return m_isInRedo; }

    void setActivePopupMenu(WebPopupMenu*);
    
    WebOpenPanelResultListener* activeOpenPanelResultListener() const { return m_activeOpenPanelResultListener.get(); }
    void setActiveOpenPanelResultListener(PassRefPtr<WebOpenPanelResultListener>);

    // -- Called from WebProcess.
    void didReceiveMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    CoreIPC::SyncReplyMode didReceiveSyncMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*, CoreIPC::ArgumentEncoder*);

    // -- InjectedBundle methods
    void initializeInjectedBundleContextMenuClient(WKBundlePageContextMenuClient*);
    void initializeInjectedBundleEditorClient(WKBundlePageEditorClient*);
    void initializeInjectedBundleFormClient(WKBundlePageFormClient*);
    void initializeInjectedBundleLoaderClient(WKBundlePageLoaderClient*);
    void initializeInjectedBundleUIClient(WKBundlePageUIClient*);

    InjectedBundlePageContextMenuClient& injectedBundleContextMenuClient() { return m_contextMenuClient; }
    InjectedBundlePageEditorClient& injectedBundleEditorClient() { return m_editorClient; }
    InjectedBundlePageFormClient& injectedBundleFormClient() { return m_formClient; }
    InjectedBundlePageLoaderClient& injectedBundleLoaderClient() { return m_loaderClient; }
    InjectedBundlePageUIClient& injectedBundleUIClient() { return m_uiClient; }

    bool findStringFromInjectedBundle(const String&, FindOptions);

    WebFrame* mainFrame() const { return m_mainFrame.get(); }
    PassRefPtr<Plugin> createPlugin(const Plugin::Parameters&);

    String renderTreeExternalRepresentation() const;
    void executeEditingCommand(const String& commandName, const String& argument);
    bool isEditingCommandEnabled(const String& commandName);
    void clearMainFrameName();
    void sendClose();

    double textZoomFactor() const;
    void setTextZoomFactor(double);
    double pageZoomFactor() const;
    void setPageZoomFactor(double);
    void setPageAndTextZoomFactors(double pageZoomFactor, double textZoomFactor);

    void scaleWebView(double scale, const WebCore::IntPoint& origin);
    double viewScaleFactor() const;

    bool drawsBackground() const { return m_drawsBackground; }
    bool drawsTransparentBackground() const { return m_drawsTransparentBackground; }

    void stopLoading();

#if USE(ACCELERATED_COMPOSITING)
    void changeAcceleratedCompositingMode(WebCore::GraphicsLayer*);
    void enterAcceleratedCompositingMode(WebCore::GraphicsLayer*);
    void exitAcceleratedCompositingMode();
#endif

#if PLATFORM(MAC)
    void addPluginView(PluginView*);
    void removePluginView(PluginView*);

    bool windowIsVisible() const { return m_windowIsVisible; }
    const WebCore::IntRect& windowFrameInScreenCoordinates() const { return m_windowFrameInScreenCoordinates; }
    const WebCore::IntRect& viewFrameInWindowCoordinates() const { return m_viewFrameInWindowCoordinates; }
    bool windowIsFocused() const;
    bool interceptEditingKeyboardEvent(WebCore::KeyboardEvent*, bool);
#elif PLATFORM(WIN)
    HWND nativeWindow() const { return m_nativeWindow; }
#endif

    void installPageOverlay(PassRefPtr<PageOverlay>);
    void uninstallPageOverlay(PageOverlay*);

    PassRefPtr<WebImage> snapshotInViewCoordinates(const WebCore::IntRect&, ImageOptions);
    PassRefPtr<WebImage> snapshotInDocumentCoordinates(const WebCore::IntRect&, ImageOptions);

    static const WebEvent* currentEvent();

    FindController& findController() { return m_findController; }

    void pageDidScroll();
#if ENABLE(TILED_BACKING_STORE)
    void pageDidRequestScroll(const WebCore::IntSize& delta);
    void setActualVisibleContentRect(const WebCore::IntRect&);

    bool resizesToContentsEnabled() const { return !m_resizesToContentsLayoutSize.isEmpty(); }
    WebCore::IntSize resizesToContentsLayoutSize() const { return m_resizesToContentsLayoutSize; }
    void setResizesToContentsUsingLayoutSize(const WebCore::IntSize& targetLayoutSize);
    void resizeToContentsIfNeeded();
#endif

    WebContextMenu* contextMenu();
    
    bool hasLocalDataForURL(const WebCore::KURL&);
    
    static bool canHandleRequest(const WebCore::ResourceRequest&);

    class SandboxExtensionTracker {
    public:
        ~SandboxExtensionTracker();

        void invalidate();

        void beginLoad(WebFrame*, const SandboxExtension::Handle& handle);
        void didStartProvisionalLoad(WebFrame*);
        void didCommitProvisionalLoad(WebFrame*);
        void didFailProvisionalLoad(WebFrame*);
    private:
        RefPtr<SandboxExtension> m_pendingProvisionalSandboxExtension;
        RefPtr<SandboxExtension> m_provisionalSandboxExtension;
        RefPtr<SandboxExtension> m_committedSandboxExtension;
    };

    SandboxExtensionTracker& sandboxExtensionTracker() { return m_sandboxExtensionTracker; }

    static void getLocationAndLengthFromRange(WebCore::Range*, uint64_t& location, uint64_t& length);

#if PLATFORM(MAC)
    void sendComplexTextInputToPlugin(uint64_t pluginComplexTextInputIdentifier, const String& textInput);

    void getMarkedRange(uint64_t& location, uint64_t& length);
    void characterIndexForPoint(const WebCore::IntPoint point, uint64_t& result);
    void firstRectForCharacterRange(uint64_t location, uint64_t length, WebCore::IntRect& resultRect);
#elif PLATFORM(WIN)
    void confirmComposition(const String& compositionString);
    void setComposition(const WTF::String& compositionString, const WTF::Vector<WebCore::CompositionUnderline>& underlines, uint64_t cursorPosition);
    void firstRectForCharacterInSelectedRange(const uint64_t characterPosition, WebCore::IntRect& resultRect);
    void getSelectedText(WTF::String&);
#endif

    // FIXME: This a dummy message, to avoid breaking the build for platforms that don't require
    // any synchronous messages, and should be removed when <rdar://problem/8775115> is fixed.
    void dummy(bool&);

#if PLATFORM(MAC)
    bool isSpeaking();
    void speak(const String&);
    void stopSpeaking();
#endif

private:
    WebPage(uint64_t pageID, const WebPageCreationParameters&);

    virtual Type type() const { return APIType; }

    void platformInitialize();

    void didReceiveWebPageMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*);
    CoreIPC::SyncReplyMode didReceiveSyncWebPageMessage(CoreIPC::Connection*, CoreIPC::MessageID, CoreIPC::ArgumentDecoder*, CoreIPC::ArgumentEncoder*);

    static const char* interpretKeyEvent(const WebCore::KeyboardEvent*);
    bool performDefaultBehaviorForKeyEvent(const WebKeyboardEvent&);

    String sourceForFrame(WebFrame*);

    void loadData(PassRefPtr<WebCore::SharedBuffer>, const String& MIMEType, const String& encodingName, const WebCore::KURL& baseURL, const WebCore::KURL& failingURL);

    bool platformHasLocalDataForURL(const WebCore::KURL&);

    // Actions
    void tryClose();
    void loadURL(const String&, const SandboxExtension::Handle& sandboxExtensionHandle);
    void loadURLRequest(const WebCore::ResourceRequest&, const SandboxExtension::Handle& sandboxExtensionHandle);
    void loadHTMLString(const String& htmlString, const String& baseURL);
    void loadAlternateHTMLString(const String& htmlString, const String& baseURL, const String& unreachableURL);
    void loadPlainTextString(const String&);
    void reload(bool reloadFromOrigin);
    void goForward(uint64_t);
    void goBack(uint64_t);
    void goToBackForwardItem(uint64_t);
    void setActive(bool);
    void setFocused(bool);
    void setInitialFocus(bool);
    void setWindowResizerSize(const WebCore::IntSize&);
    void setIsInWindow(bool);
    void mouseEvent(const WebMouseEvent&);
    void wheelEvent(const WebWheelEvent&);
    void keyEvent(const WebKeyboardEvent&);
    void validateMenuItem(const String&);
    void executeEditCommand(const String&);
#if ENABLE(TOUCH_EVENTS)
    void touchEvent(const WebTouchEvent&);
#endif

    void setDrawsBackground(bool);
    void setDrawsTransparentBackground(bool);

    void getContentsAsString(uint64_t callbackID);
    void getMainResourceDataOfFrame(uint64_t frameID, uint64_t callbackID);
    void getRenderTreeExternalRepresentation(uint64_t callbackID);
    void getSelectionOrContentsAsString(uint64_t callbackID);
    void getSourceForFrame(uint64_t frameID, uint64_t callbackID);
    void getWebArchiveOfFrame(uint64_t frameID, uint64_t callbackID);
    void runJavaScriptInMainFrame(const String&, uint64_t callbackID);

    void preferencesDidChange(const WebPreferencesStore&);
    void platformPreferencesDidChange(const WebPreferencesStore&);
    void updatePreferences(const WebPreferencesStore&);

    void didReceivePolicyDecision(uint64_t frameID, uint64_t listenerID, uint32_t policyAction, uint64_t downloadID);
    void setUserAgent(const String&);
    void setCustomTextEncodingName(const String&);

#if PLATFORM(MAC)
    void setWindowIsVisible(bool windowIsVisible);
    void windowAndViewFramesChanged(const WebCore::IntRect& windowFrameInScreenCoordinates, const WebCore::IntRect& viewFrameInWindowCoordinates);
#endif

    void unapplyEditCommand(uint64_t commandID);
    void reapplyEditCommand(uint64_t commandID);
    void didRemoveEditCommand(uint64_t commandID);

    void findString(const String&, uint32_t findOptions, uint32_t maxMatchCount);
    void hideFindUI();
    void countStringMatches(const String&, uint32_t findOptions, uint32_t maxMatchCount);

#if PLATFORM(QT)
    void findZoomableAreaForPoint(const WebCore::IntPoint&);
#endif

    void didChangeSelectedIndexForActivePopupMenu(int32_t newIndex);
    void setTextForActivePopupMenu(int32_t index);

    void didChooseFilesForOpenPanel(const Vector<String>&);
    void didCancelForOpenPanel();

    void unmarkAllMisspellings();
    void unmarkAllBadGrammar();

#if ENABLE(CONTEXT_MENUS)
    void didSelectItemFromActiveContextMenu(const WebContextMenuItemData&);
#endif

    OwnPtr<WebCore::Page> m_page;
    RefPtr<WebFrame> m_mainFrame;
    RefPtr<InjectedBundleBackForwardList> m_backForwardList;

    RefPtr<WebPageGroupProxy> m_pageGroup;

    String m_userAgent;

    WebCore::IntSize m_viewSize;
    RefPtr<DrawingArea> m_drawingArea;

    bool m_drawsBackground;
    bool m_drawsTransparentBackground;

    bool m_isInRedo;
    bool m_isClosed;

    bool m_tabToLinks;

#if PLATFORM(MAC)
    // Whether the containing window is visible or not.
    bool m_windowIsVisible;

    // The frame of the containing window in screen coordinates.
    WebCore::IntRect m_windowFrameInScreenCoordinates;

    // The frame of the view in window coordinates.
    WebCore::IntRect m_viewFrameInWindowCoordinates;

    // All plug-in views on this web page.
    HashSet<PluginView*> m_pluginViews;
#elif PLATFORM(WIN)
    // Our view's window (in the UI process).
    HWND m_nativeWindow;
#endif
    
    HashMap<uint64_t, RefPtr<WebEditCommand> > m_editCommandMap;

    WebCore::IntSize m_windowResizerSize;

    InjectedBundlePageContextMenuClient m_contextMenuClient;
    InjectedBundlePageEditorClient m_editorClient;
    InjectedBundlePageFormClient m_formClient;
    InjectedBundlePageLoaderClient m_loaderClient;
    InjectedBundlePageUIClient m_uiClient;

#if ENABLE(TILED_BACKING_STORE)
    WebCore::IntSize m_resizesToContentsLayoutSize;
#endif

    FindController m_findController;
    RefPtr<PageOverlay> m_pageOverlay;

#if ENABLE(INSPECTOR)
    OwnPtr<WebInspector> m_inspector;
#endif
    RefPtr<WebPopupMenu> m_activePopupMenu;
    RefPtr<WebContextMenu> m_contextMenu;
    RefPtr<WebOpenPanelResultListener> m_activeOpenPanelResultListener;

    SandboxExtensionTracker m_sandboxExtensionTracker;
    uint64_t m_pageID;
};

} // namespace WebKit

#endif // WebPage_h
