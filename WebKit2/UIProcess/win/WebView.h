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

#ifndef WebView_h
#define WebView_h

#include "APIObject.h"
#include "PageClient.h"
#include "WebPageProxy.h"
#include <WebCore/WindowMessageListener.h>
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>

namespace WebKit {

class DrawingAreaProxy;

class WebView : public APIObject, public PageClient, WebCore::WindowMessageListener {
public:
    static PassRefPtr<WebView> create(RECT rect, WebContext* context, WebPageGroup* pageGroup, HWND parentWindow)
    {
        return adoptRef(new WebView(rect, context, pageGroup, parentWindow));
    }
    ~WebView();

    RECT rect() const { return m_rect; }

    HWND window() const { return m_window; }
    void setParentWindow(HWND);
    void windowAncestryDidChange();
    void setIsInWindow(bool);
    void setOverrideCursor(HCURSOR overrideCursor);
    void setInitialFocus(bool forward);

    WebPageProxy* page() const { return m_page.get(); }

private:
    WebView(RECT, WebContext*, WebPageGroup*, HWND parentWindow);

    virtual Type type() const { return TypeView; }

    static bool registerWebViewWindowClass();
    static LRESULT CALLBACK WebViewWndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT onMouseEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onWheelEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onKeyEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onPaintEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onPrintClientEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onSizeEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onWindowPositionChangedEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onSetFocusEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onKillFocusEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onTimerEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onShowWindowEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    LRESULT onSetCursor(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled);
    bool onIMEStartComposition();
    bool onIMEComposition(LPARAM);
    bool onIMEEndComposition();
    LRESULT onIMERequest(WPARAM, LPARAM);
    bool onIMESelect(WPARAM, LPARAM);
    bool onIMESetContext(WPARAM, LPARAM);
    void resetIME();
    void setInputMethodState(bool);
    HIMC getIMMContext();
    void prepareCandidateWindow(HIMC);
    LRESULT onIMERequestCharPosition(IMECHARPOSITION*);
    LRESULT onIMERequestReconvertString(RECONVERTSTRING*);

    bool isActive();
    void updateActiveState();
    void updateActiveStateSoon();

    void initializeToolTipWindow();

    void startTrackingMouseLeave();
    void stopTrackingMouseLeave();

    void close();

    void updateNativeCursor();

    // PageClient
    virtual void processDidCrash();
    virtual void didRelaunchProcess();
    virtual void takeFocus(bool direction);
    virtual void toolTipChanged(const WTF::String&, const WTF::String&);
    virtual void setCursor(const WebCore::Cursor&);
    virtual void setViewportArguments(const WebCore::ViewportArguments&);
    virtual void registerEditCommand(PassRefPtr<WebEditCommandProxy>, WebPageProxy::UndoOrRedo);
    virtual void clearAllEditCommands();
    virtual void setEditCommandState(const WTF::String&, bool, int);
    virtual WebCore::FloatRect convertToDeviceSpace(const WebCore::FloatRect&);
    virtual WebCore::FloatRect convertToUserSpace(const WebCore::FloatRect&);
    virtual void didNotHandleKeyEvent(const NativeWebKeyboardEvent&);
    virtual void compositionSelectionChanged(bool);
    virtual PassRefPtr<WebPopupMenuProxy> createPopupMenuProxy(WebPageProxy*);
    virtual PassRefPtr<WebContextMenuProxy> createContextMenuProxy(WebPageProxy*);
    virtual void setFindIndicator(PassRefPtr<FindIndicator>, bool fadeOut);

#if USE(ACCELERATED_COMPOSITING)
    virtual void pageDidEnterAcceleratedCompositing();
    virtual void pageDidLeaveAcceleratedCompositing();
    void switchToDrawingAreaTypeIfNecessary(DrawingAreaInfo::Type);
#endif

    void didCommitLoadForMainFrame(bool useCustomRepresentation);
    void didFinishLoadingDataForCustomRepresentation(const CoreIPC::DataReference&);

    virtual HWND nativeWindow();

    // WebCore::WindowMessageListener
    virtual void windowReceivedMessage(HWND, UINT message, WPARAM, LPARAM);

    RECT m_rect;
    HWND m_window;
    HWND m_topLevelParentWindow;
    HWND m_toolTipWindow;

    HCURSOR m_lastCursorSet;
    HCURSOR m_webCoreCursor;
    HCURSOR m_overrideCursor;

    bool m_trackingMouseLeave;
    bool m_isBeingDestroyed;

    RefPtr<WebPageProxy> m_page;

    unsigned m_inIMEComposition;
};

} // namespace WebKit

#endif // WebView_h
