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

#include "WebView.h"

#include "ChunkedUpdateDrawingAreaProxy.h"
#include "FindIndicator.h"
#include "LayerBackedDrawingAreaProxy.h"
#include "Logging.h"
#include "RunLoop.h"
#include "NativeWebKeyboardEvent.h"
#include "WebContext.h"
#include "WebContextMenuProxyWin.h"
#include "WebEditCommandProxy.h"
#include "WebEventFactory.h"
#include "WebPageProxy.h"
#include "WebPopupMenuProxyWin.h"
#include <Commctrl.h>
#include <WebCore/Cursor.h>
#include <WebCore/FloatRect.h>
#include <WebCore/IntRect.h>
#include <WebCore/SoftLinking.h>
#include <WebCore/WebCoreInstanceHandle.h>
#include <WebCore/WindowMessageBroadcaster.h>
#include <wtf/text/WTFString.h>

namespace Ime {
    // We need these functions in a separate namespace, because in the global namespace they conflict
    // with the definitions in imm.h only by the type modifier (the macro defines them as static) and
    // imm.h is included by windows.h
    SOFT_LINK_LIBRARY(IMM32)
    SOFT_LINK(IMM32, ImmGetContext, HIMC, WINAPI, (HWND hwnd), (hwnd))
    SOFT_LINK(IMM32, ImmReleaseContext, BOOL, WINAPI, (HWND hWnd, HIMC hIMC), (hWnd, hIMC))
    SOFT_LINK(IMM32, ImmGetCompositionStringW, LONG, WINAPI, (HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen), (hIMC, dwIndex, lpBuf, dwBufLen))
    SOFT_LINK(IMM32, ImmSetCandidateWindow, BOOL, WINAPI, (HIMC hIMC, LPCANDIDATEFORM lpCandidate), (hIMC, lpCandidate))
    SOFT_LINK(IMM32, ImmSetOpenStatus, BOOL, WINAPI, (HIMC hIMC, BOOL fOpen), (hIMC, fOpen))
    SOFT_LINK(IMM32, ImmNotifyIME, BOOL, WINAPI, (HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue), (hIMC, dwAction, dwIndex, dwValue))
    SOFT_LINK(IMM32, ImmAssociateContextEx, BOOL, WINAPI, (HWND hWnd, HIMC hIMC, DWORD dwFlags), (hWnd, hIMC, dwFlags))
};

using namespace WebCore;

namespace WebKit {

static const LPCWSTR kWebKit2WebViewWindowClassName = L"WebKit2WebViewWindowClass";

// Constants not available on all platforms.
const int WM_XP_THEMECHANGED = 0x031A;
const int WM_VISTA_MOUSEHWHEEL = 0x020E;

static const int kMaxToolTipWidth = 250;

enum {
    UpdateActiveStateTimer = 1,
};

LRESULT CALLBACK WebView::WebViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR longPtr = ::GetWindowLongPtr(hWnd, 0);
    
    if (WebView* webView = reinterpret_cast<WebView*>(longPtr))
        return webView->wndProc(hWnd, message, wParam, lParam);

    if (message == WM_CREATE) {
        LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

        // Associate the WebView with the window.
        ::SetWindowLongPtr(hWnd, 0, (LONG_PTR)createStruct->lpCreateParams);
        return 0;
    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WebView::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    bool handled = true;

    switch (message) {
        case WM_CLOSE:
            m_page->tryClose();
            break;
        case WM_DESTROY:
            m_isBeingDestroyed = true;
            close();
            break;
        case WM_ERASEBKGND:
            lResult = 1;
            break;
        case WM_PAINT:
            lResult = onPaintEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_PRINTCLIENT:
            lResult = onPrintClientEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSELEAVE:
            lResult = onMouseEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_MOUSEWHEEL:
        case WM_VISTA_MOUSEHWHEEL:
            lResult = onWheelEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSCHAR:
        case WM_CHAR:
        case WM_SYSKEYUP:
        case WM_KEYUP:
            lResult = onKeyEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_SIZE:
            lResult = onSizeEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_WINDOWPOSCHANGED:
            lResult = onWindowPositionChangedEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_SETFOCUS:
            lResult = onSetFocusEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_KILLFOCUS:
            lResult = onKillFocusEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_TIMER:
            lResult = onTimerEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_SHOWWINDOW:
            lResult = onShowWindowEvent(hWnd, message, wParam, lParam, handled);
            break;
        case WM_SETCURSOR:
            lResult = onSetCursor(hWnd, message, wParam, lParam, handled);
            break;
        case WM_IME_STARTCOMPOSITION:
            handled = onIMEStartComposition();
            break;
        case WM_IME_REQUEST:
            lResult = onIMERequest(wParam, lParam);
            break;
        case WM_IME_COMPOSITION:
            handled = onIMEComposition(lParam);
            break;
        case WM_IME_ENDCOMPOSITION:
            handled = onIMEEndComposition();
            break;
        case WM_IME_SELECT:
            handled = onIMESelect(wParam, lParam);
            break;
        case WM_IME_SETCONTEXT:
            handled = onIMESetContext(wParam, lParam);
            break;
        default:
            handled = false;
            break;
    }

    if (!handled)
        lResult = ::DefWindowProc(hWnd, message, wParam, lParam);

    return lResult;
}

bool WebView::registerWebViewWindowClass()
{
    static bool haveRegisteredWindowClass = false;
    if (haveRegisteredWindowClass)
        return true;
    haveRegisteredWindowClass = true;

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_DBLCLKS;
    wcex.lpfnWndProc    = WebView::WebViewWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = sizeof(WebView*);
    wcex.hInstance      = instanceHandle();
    wcex.hIcon          = 0;
    wcex.hCursor        = ::LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground  = 0;
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = kWebKit2WebViewWindowClassName;
    wcex.hIconSm        = 0;

    return !!::RegisterClassEx(&wcex);
}

WebView::WebView(RECT rect, WebContext* context, WebPageGroup* pageGroup, HWND parentWindow)
    : m_rect(rect)
    , m_topLevelParentWindow(0)
    , m_toolTipWindow(0)
    , m_lastCursorSet(0)
    , m_webCoreCursor(0)
    , m_overrideCursor(0)
    , m_trackingMouseLeave(false)
    , m_isBeingDestroyed(false)
    , m_inIMEComposition(0)
{
    registerWebViewWindowClass();

    m_page = context->createWebPage(pageGroup);
    m_page->setPageClient(this);
    m_page->setDrawingArea(ChunkedUpdateDrawingAreaProxy::create(this, m_page.get()));

    m_window = ::CreateWindowEx(0, kWebKit2WebViewWindowClassName, 0, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        m_rect.top, m_rect.left, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, parentWindow ? parentWindow : HWND_MESSAGE, 0, instanceHandle(), this);
    ASSERT(::IsWindow(m_window));

    m_page->initializeWebPage(IntRect(m_rect).size());

    ::ShowWindow(m_window, SW_SHOW);

    // FIXME: Initializing the tooltip window here matches WebKit win, but seems like something
    // we could do on demand to save resources.
    initializeToolTipWindow();

    // Initialize the top level parent window and register it with the WindowMessageBroadcaster.
    windowAncestryDidChange();
}

WebView::~WebView()
{
    // Tooltip window needs to be explicitly destroyed since it isn't a WS_CHILD.
    if (::IsWindow(m_toolTipWindow))
        ::DestroyWindow(m_toolTipWindow);
}

void WebView::setParentWindow(HWND parentWindow)
{
    if (m_window) {
        // If the host window hasn't changed, bail.
        if (::GetParent(m_window) == parentWindow)
            return;
        if (parentWindow)
            ::SetParent(m_window, parentWindow);
        else if (!m_isBeingDestroyed) {
            // Turn the WebView into a message-only window so it will no longer be a child of the
            // old parent window and will be hidden from screen. We only do this when
            // isBeingDestroyed() is false because doing this while handling WM_DESTROY can leave
            // m_window in a weird state (see <http://webkit.org/b/29337>).
            ::SetParent(m_window, HWND_MESSAGE);
        }
    }

    windowAncestryDidChange();
}

static HWND findTopLevelParentWindow(HWND window)
{
    if (!window)
        return 0;

    HWND current = window;
    for (HWND parent = GetParent(current); current; current = parent, parent = GetParent(parent)) {
        if (!parent || !(GetWindowLongPtr(current, GWL_STYLE) & (WS_POPUP | WS_CHILD)))
            return current;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

void WebView::windowAncestryDidChange()
{
    HWND newTopLevelParentWindow;
    if (m_window)
        newTopLevelParentWindow = findTopLevelParentWindow(m_window);
    else {
        // There's no point in tracking active state changes of our parent window if we don't have
        // a window ourselves.
        newTopLevelParentWindow = 0;
    }

    if (newTopLevelParentWindow == m_topLevelParentWindow)
        return;

    if (m_topLevelParentWindow)
        WindowMessageBroadcaster::removeListener(m_topLevelParentWindow, this);

    m_topLevelParentWindow = newTopLevelParentWindow;

    if (m_topLevelParentWindow)
        WindowMessageBroadcaster::addListener(m_topLevelParentWindow, this);

    updateActiveState();
}

LRESULT WebView::onMouseEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
{
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            ::SetFocus(m_window);
            ::SetCapture(m_window);
            break; 
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            ::ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            startTrackingMouseLeave();
            break;
        case WM_MOUSELEAVE:
            stopTrackingMouseLeave();
            break;
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
            break;
        default:
            ASSERT_NOT_REACHED();
    }

    WebMouseEvent mouseEvent = WebEventFactory::createWebMouseEvent(hWnd, message, wParam, lParam);
    m_page->handleMouseEvent(mouseEvent);

    handled = true;
    return 0;
}

LRESULT WebView::onWheelEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
{
    WebWheelEvent wheelEvent = WebEventFactory::createWebWheelEvent(hWnd, message, wParam, lParam);
    if (wheelEvent.controlKey()) {
        // We do not want WebKit to handle Control + Wheel, this should be handled by the client application
        // to zoom the page.
        handled = false;
        return 0;
    }

    m_page->handleWheelEvent(wheelEvent);

    handled = true;
    return 0;
}

LRESULT WebView::onKeyEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
{
    m_page->handleKeyboardEvent(NativeWebKeyboardEvent(hWnd, message, wParam, lParam));

    // We claim here to always have handled the event. If the event is not in fact handled, we will
    // find out later in didNotHandleKeyEvent.
    handled = true;
    return 0;
}

LRESULT WebView::onPaintEvent(HWND hWnd, UINT message, WPARAM, LPARAM, bool& handled)
{
    PAINTSTRUCT paintStruct;
    HDC hdc = ::BeginPaint(m_window, &paintStruct);

    m_page->drawingArea()->paint(IntRect(paintStruct.rcPaint), hdc);

    ::EndPaint(m_window, &paintStruct);

    handled = true;
    return 0;
}

LRESULT WebView::onPrintClientEvent(HWND hWnd, UINT, WPARAM wParam, LPARAM, bool& handled)
{
    HDC hdc = reinterpret_cast<HDC>(wParam);
    RECT winRect;
    ::GetClientRect(hWnd, &winRect);
    IntRect rect = winRect;

    m_page->drawingArea()->paint(rect, hdc);

    handled = true;
    return 0;
}

LRESULT WebView::onSizeEvent(HWND, UINT, WPARAM, LPARAM lParam, bool& handled)
{
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);

    m_page->drawingArea()->setSize(IntSize(width, height));

    handled = true;
    return 0;
}

LRESULT WebView::onWindowPositionChangedEvent(HWND, UINT, WPARAM, LPARAM lParam, bool& handled)
{
    if (reinterpret_cast<WINDOWPOS*>(lParam)->flags & SWP_SHOWWINDOW)
        updateActiveStateSoon();

    handled = false;
    return 0;
}

LRESULT WebView::onSetFocusEvent(HWND, UINT, WPARAM, LPARAM lParam, bool& handled)
{
    m_page->setFocused(true);

    handled = true;
    return 0;
}

LRESULT WebView::onKillFocusEvent(HWND, UINT, WPARAM, LPARAM lParam, bool& handled)
{
    m_page->setFocused(false);

    handled = true;
    return 0;
}

LRESULT WebView::onTimerEvent(HWND hWnd, UINT, WPARAM wParam, LPARAM, bool& handled)
{
    switch (wParam) {
        case UpdateActiveStateTimer:
            ::KillTimer(hWnd, UpdateActiveStateTimer);
            updateActiveState();
            break;
    }

    handled = true;
    return 0;
}

LRESULT WebView::onShowWindowEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
{
    // lParam is 0 when the message is sent because of a ShowWindow call.
    // FIXME: Is WM_SHOWWINDOW sent when ShowWindow is called on an ancestor of our window?
    if (!lParam) {
        bool isVisible = wParam;

        // Notify the drawing area that the visibility changed.
        m_page->drawingArea()->setPageIsVisible(isVisible);

        handled = true;
    }

    return 0;
}

LRESULT WebView::onSetCursor(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
{
    handled = ::SetCursor(m_lastCursorSet);
    return 0;
}

bool WebView::isActive()
{
    HWND activeWindow = ::GetActiveWindow();
    return (activeWindow && m_topLevelParentWindow == findTopLevelParentWindow(activeWindow));
}

void WebView::updateActiveState()
{
    m_page->setActive(isActive());
}

void WebView::updateActiveStateSoon()
{
    // This function is called while processing the WM_NCACTIVATE message.
    // While processing WM_NCACTIVATE when we are being deactivated, GetActiveWindow() will
    // still return our window. If we were to call updateActiveState() in that case, we would
    // wrongly think that we are still the active window. To work around this, we update our
    // active state after a 0-delay timer fires, at which point GetActiveWindow() will return
    // the newly-activated window.

    ::SetTimer(m_window, UpdateActiveStateTimer, 0, 0);
}

static bool initCommonControls()
{
    static bool haveInitialized = false;
    if (haveInitialized)
        return true;

    INITCOMMONCONTROLSEX init;
    init.dwSize = sizeof(init);
    init.dwICC = ICC_TREEVIEW_CLASSES;
    haveInitialized = !!::InitCommonControlsEx(&init);
    return haveInitialized;
}

void WebView::initializeToolTipWindow()
{
    if (!initCommonControls())
        return;

    m_toolTipWindow = ::CreateWindowEx(WS_EX_TRANSPARENT, TOOLTIPS_CLASS, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       m_window, 0, 0, 0);
    if (!m_toolTipWindow)
        return;

    TOOLINFO info = {0};
    info.cbSize = sizeof(info);
    info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    info.uId = reinterpret_cast<UINT_PTR>(m_window);

    ::SendMessage(m_toolTipWindow, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&info));
    ::SendMessage(m_toolTipWindow, TTM_SETMAXTIPWIDTH, 0, kMaxToolTipWidth);
    ::SetWindowPos(m_toolTipWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void WebView::startTrackingMouseLeave()
{
    if (m_trackingMouseLeave)
        return;
    m_trackingMouseLeave = true;

    TRACKMOUSEEVENT trackMouseEvent;
    trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    trackMouseEvent.dwFlags = TME_LEAVE;
    trackMouseEvent.hwndTrack = m_window;

    ::TrackMouseEvent(&trackMouseEvent);
}

void WebView::stopTrackingMouseLeave()
{
    if (!m_trackingMouseLeave)
        return;
    m_trackingMouseLeave = false;

    TRACKMOUSEEVENT trackMouseEvent;
    trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
    trackMouseEvent.dwFlags = TME_LEAVE | TME_CANCEL;
    trackMouseEvent.hwndTrack = m_window;

    ::TrackMouseEvent(&trackMouseEvent);
}

void WebView::close()
{
    setParentWindow(0);
    m_page->close();
}

// PageClient

void WebView::processDidCrash()
{
}

void WebView::didRelaunchProcess()
{
}

void WebView::takeFocus(bool)
{
}

void WebView::toolTipChanged(const String&, const String& newToolTip)
{
    if (!m_toolTipWindow)
        return;

    if (!newToolTip.isEmpty()) {
        // This is necessary because String::charactersWithNullTermination() is not const.
        String toolTip = newToolTip;

        TOOLINFO info = {0};
        info.cbSize = sizeof(info);
        info.uFlags = TTF_IDISHWND;
        info.uId = reinterpret_cast<UINT_PTR>(m_window);
        info.lpszText = const_cast<UChar*>(toolTip.charactersWithNullTermination());
        ::SendMessage(m_toolTipWindow, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&info));
    }

    ::SendMessage(m_toolTipWindow, TTM_ACTIVATE, !newToolTip.isEmpty(), 0);
}

void WebView::updateNativeCursor()
{
    // We only show the override cursor if the default (arrow) cursor is showing.
    static HCURSOR arrowCursor = ::LoadCursor(0, IDC_ARROW);
    if (m_overrideCursor && m_webCoreCursor == arrowCursor)
        m_lastCursorSet = m_overrideCursor;
    else
        m_lastCursorSet = m_webCoreCursor;

    ::SetCursor(m_lastCursorSet);
}

void WebView::setCursor(const WebCore::Cursor& cursor)
{
    if (!cursor.platformCursor()->nativeCursor())
        return;
    m_webCoreCursor = cursor.platformCursor()->nativeCursor();
    updateNativeCursor();
}

void WebView::setOverrideCursor(HCURSOR overrideCursor)
{
    m_overrideCursor = overrideCursor;
    updateNativeCursor();
}

void WebView::setInitialFocus(bool forward)
{
    m_page->setInitialFocus(forward);
}

void WebView::setViewportArguments(const WebCore::ViewportArguments&)
{
}

void WebView::registerEditCommand(PassRefPtr<WebEditCommandProxy>, WebPageProxy::UndoOrRedo)
{
}

void WebView::clearAllEditCommands()
{
}

void WebView::setEditCommandState(const String&, bool, int)
{
}

FloatRect WebView::convertToDeviceSpace(const FloatRect& rect)
{
    return rect;
}

FloatRect WebView::convertToUserSpace(const FloatRect& rect)
{
    return rect;
}

HIMC WebView::getIMMContext() 
{
    return Ime::ImmGetContext(m_window);
}

void WebView::prepareCandidateWindow(HIMC hInputContext) 
{
    IntRect caret = m_page->firstRectForCharacterInSelectedRange(0);
    CANDIDATEFORM form;
    form.dwIndex = 0;
    form.dwStyle = CFS_EXCLUDE;
    form.ptCurrentPos.x = caret.x();
    form.ptCurrentPos.y = caret.bottom();
    form.rcArea.top = caret.y();
    form.rcArea.bottom = caret.bottom();
    form.rcArea.left = caret.x();
    form.rcArea.right = caret.right();
    Ime::ImmSetCandidateWindow(hInputContext, &form);
}

void WebView::resetIME()
{
    HIMC hInputContext = getIMMContext();
    if (!hInputContext)
        return;
    Ime::ImmNotifyIME(hInputContext, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    Ime::ImmReleaseContext(m_window, hInputContext);
}

void WebView::setInputMethodState(bool enabled)
{
    Ime::ImmAssociateContextEx(m_window, 0, enabled ? IACE_DEFAULT : 0);
}

void WebView::compositionSelectionChanged(bool hasChanged)
{
    if (m_page->selectionState().hasComposition && !hasChanged)
        resetIME();
}

bool WebView::onIMEStartComposition()
{
    LOG(TextInput, "onIMEStartComposition");
    m_inIMEComposition++;

    HIMC hInputContext = getIMMContext();
    if (!hInputContext)
        return false;
    prepareCandidateWindow(hInputContext);
    Ime::ImmReleaseContext(m_window, hInputContext);
    return true;
}

static bool getCompositionString(HIMC hInputContext, DWORD type, String& result)
{
    LONG compositionLength = Ime::ImmGetCompositionStringW(hInputContext, type, 0, 0);
    if (compositionLength <= 0)
        return false;
    Vector<UChar> compositionBuffer(compositionLength / 2);
    compositionLength = Ime::ImmGetCompositionStringW(hInputContext, type, compositionBuffer.data(), compositionLength);
    result = String::adopt(compositionBuffer);
    return true;
}

static void compositionToUnderlines(const Vector<DWORD>& clauses, const Vector<BYTE>& attributes, Vector<CompositionUnderline>& underlines)
{
    if (clauses.isEmpty()) {
        underlines.clear();
        return;
    }
  
    size_t numBoundaries = clauses.size() - 1;
    underlines.resize(numBoundaries);
    for (unsigned i = 0; i < numBoundaries; ++i) {
        underlines[i].startOffset = clauses[i];
        underlines[i].endOffset = clauses[i + 1];
        BYTE attribute = attributes[clauses[i]];
        underlines[i].thick = attribute == ATTR_TARGET_CONVERTED || attribute == ATTR_TARGET_NOTCONVERTED;
        underlines[i].color = Color::black;
    }
}

#if !LOG_DISABLED
#define APPEND_ARGUMENT_NAME(name) \
    if (lparam & name) { \
        if (needsComma) \
            result += ", "; \
            result += #name; \
        needsComma = true; \
    }

static String imeCompositionArgumentNames(LPARAM lparam)
{
    String result;
    bool needsComma = false;

    APPEND_ARGUMENT_NAME(GCS_COMPATTR);
    APPEND_ARGUMENT_NAME(GCS_COMPCLAUSE);
    APPEND_ARGUMENT_NAME(GCS_COMPREADSTR);
    APPEND_ARGUMENT_NAME(GCS_COMPREADATTR);
    APPEND_ARGUMENT_NAME(GCS_COMPREADCLAUSE);
    APPEND_ARGUMENT_NAME(GCS_COMPSTR);
    APPEND_ARGUMENT_NAME(GCS_CURSORPOS);
    APPEND_ARGUMENT_NAME(GCS_DELTASTART);
    APPEND_ARGUMENT_NAME(GCS_RESULTCLAUSE);
    APPEND_ARGUMENT_NAME(GCS_RESULTREADCLAUSE);
    APPEND_ARGUMENT_NAME(GCS_RESULTREADSTR);
    APPEND_ARGUMENT_NAME(GCS_RESULTSTR);
    APPEND_ARGUMENT_NAME(CS_INSERTCHAR);
    APPEND_ARGUMENT_NAME(CS_NOMOVECARET);

    return result;
}

static String imeRequestName(WPARAM wparam)
{
    switch (wparam) {
    case IMR_CANDIDATEWINDOW:
        return "IMR_CANDIDATEWINDOW";
    case IMR_COMPOSITIONFONT:
        return "IMR_COMPOSITIONFONT";
    case IMR_COMPOSITIONWINDOW:
        return "IMR_COMPOSITIONWINDOW";
    case IMR_CONFIRMRECONVERTSTRING:
        return "IMR_CONFIRMRECONVERTSTRING";
    case IMR_DOCUMENTFEED:
        return "IMR_DOCUMENTFEED";
    case IMR_QUERYCHARPOSITION:
        return "IMR_QUERYCHARPOSITION";
    case IMR_RECONVERTSTRING:
        return "IMR_RECONVERTSTRING";
    default:
        return "Unknown (" + String::number(wparam) + ")";
    }
}
#endif

bool WebView::onIMEComposition(LPARAM lparam)
{
    LOG(TextInput, "onIMEComposition %s", imeCompositionArgumentNames(lparam).latin1().data());
    HIMC hInputContext = getIMMContext();
    if (!hInputContext)
        return true;

    if (!m_page->selectionState().isContentEditable)
        return true;

    prepareCandidateWindow(hInputContext);

    if (lparam & GCS_RESULTSTR || !lparam) {
        String compositionString;
        if (!getCompositionString(hInputContext, GCS_RESULTSTR, compositionString) && lparam)
            return true;
        
        m_page->confirmComposition(compositionString);
        return true;
    }

    String compositionString;
    if (!getCompositionString(hInputContext, GCS_COMPSTR, compositionString))
        return true;
    
    // Composition string attributes
    int numAttributes = Ime::ImmGetCompositionStringW(hInputContext, GCS_COMPATTR, 0, 0);
    Vector<BYTE> attributes(numAttributes);
    Ime::ImmGetCompositionStringW(hInputContext, GCS_COMPATTR, attributes.data(), numAttributes);

    // Get clauses
    int numBytes = Ime::ImmGetCompositionStringW(hInputContext, GCS_COMPCLAUSE, 0, 0);
    Vector<DWORD> clauses(numBytes / sizeof(DWORD));
    Ime::ImmGetCompositionStringW(hInputContext, GCS_COMPCLAUSE, clauses.data(), numBytes);

    Vector<CompositionUnderline> underlines;
    compositionToUnderlines(clauses, attributes, underlines);

    int cursorPosition = LOWORD(Ime::ImmGetCompositionStringW(hInputContext, GCS_CURSORPOS, 0, 0));

    m_page->setComposition(compositionString, underlines, cursorPosition);

    return true;
}

bool WebView::onIMEEndComposition()
{
    LOG(TextInput, "onIMEEndComposition");
    // If the composition hasn't been confirmed yet, it needs to be cancelled.
    // This happens after deleting the last character from inline input hole.
    if (m_page->selectionState().hasComposition)
        m_page->confirmComposition(String());

    if (m_inIMEComposition)
        m_inIMEComposition--;

    return true;
}

LRESULT WebView::onIMERequestCharPosition(IMECHARPOSITION* charPos)
{
    if (charPos->dwCharPos && !m_page->selectionState().hasComposition)
        return 0;
    IntRect caret = m_page->firstRectForCharacterInSelectedRange(charPos->dwCharPos);
    charPos->pt.x = caret.x();
    charPos->pt.y = caret.y();
    ::ClientToScreen(m_window, &charPos->pt);
    charPos->cLineHeight = caret.height();
    ::GetWindowRect(m_window, &charPos->rcDocument);
    return true;
}

LRESULT WebView::onIMERequestReconvertString(RECONVERTSTRING* reconvertString)
{
    String text = m_page->getSelectedText();
    unsigned totalSize = sizeof(RECONVERTSTRING) + text.length() * sizeof(UChar);
    
    if (!reconvertString)
        return totalSize;

    if (totalSize > reconvertString->dwSize)
        return 0;
    reconvertString->dwCompStrLen = text.length();
    reconvertString->dwStrLen = text.length();
    reconvertString->dwTargetStrLen = text.length();
    reconvertString->dwStrOffset = sizeof(RECONVERTSTRING);
    memcpy(reconvertString + 1, text.characters(), text.length() * sizeof(UChar));
    return totalSize;
}

LRESULT WebView::onIMERequest(WPARAM request, LPARAM data)
{
    LOG(TextInput, "onIMERequest %s", imeRequestName(request).latin1().data());
    if (!m_page->selectionState().isContentEditable)
        return 0;

    switch (request) {
        case IMR_RECONVERTSTRING:
            return onIMERequestReconvertString(reinterpret_cast<RECONVERTSTRING*>(data));

        case IMR_QUERYCHARPOSITION:
            return onIMERequestCharPosition(reinterpret_cast<IMECHARPOSITION*>(data));
    }
    return 0;
}

bool WebView::onIMESelect(WPARAM wparam, LPARAM lparam)
{
    UNUSED_PARAM(wparam);
    UNUSED_PARAM(lparam);
    LOG(TextInput, "onIMESelect locale %ld %s", lparam, wparam ? "select" : "deselect");
    return false;
}

bool WebView::onIMESetContext(WPARAM wparam, LPARAM)
{
    LOG(TextInput, "onIMESetContext %s", wparam ? "active" : "inactive");
    return false;
}

void WebView::didNotHandleKeyEvent(const NativeWebKeyboardEvent& event)
{
    // Calling ::DefWindowProcW will ensure that pressing the Alt key will generate a WM_SYSCOMMAND
    // event, e.g. See <http://webkit.org/b/47671>.
    ::DefWindowProcW(event.nativeEvent()->hwnd, event.nativeEvent()->message, event.nativeEvent()->wParam, event.nativeEvent()->lParam);
}

PassRefPtr<WebPopupMenuProxy> WebView::createPopupMenuProxy(WebPageProxy* page)
{
    return WebPopupMenuProxyWin::create(this, page);
}

PassRefPtr<WebContextMenuProxy> WebView::createContextMenuProxy(WebPageProxy* page)
{
    return WebContextMenuProxyWin::create(m_window, page);
}

void WebView::setFindIndicator(PassRefPtr<FindIndicator>, bool fadeOut)
{
    // FIXME: Implement.
}

void WebView::didCommitLoadForMainFrame(bool useCustomRepresentation)
{
}

void WebView::didFinishLoadingDataForCustomRepresentation(const CoreIPC::DataReference&)
{
}

void WebView::setIsInWindow(bool isInWindow)
{
    m_page->setIsInWindow(isInWindow);
}

#if USE(ACCELERATED_COMPOSITING)

void WebView::pageDidEnterAcceleratedCompositing()
{
    switchToDrawingAreaTypeIfNecessary(DrawingAreaInfo::LayerBacked);
}

void WebView::pageDidLeaveAcceleratedCompositing()
{
    switchToDrawingAreaTypeIfNecessary(DrawingAreaInfo::ChunkedUpdate);
}

void WebView::switchToDrawingAreaTypeIfNecessary(DrawingAreaInfo::Type type)
{
    DrawingAreaInfo::Type existingDrawingAreaType = m_page->drawingArea() ? m_page->drawingArea()->info().type : DrawingAreaInfo::None;
    if (existingDrawingAreaType == type)
        return;

    OwnPtr<DrawingAreaProxy> newDrawingArea;
    switch (type) {
        case DrawingAreaInfo::None:
            break;
        case DrawingAreaInfo::ChunkedUpdate:
            newDrawingArea = ChunkedUpdateDrawingAreaProxy::create(this, m_page.get());
            break;
        case DrawingAreaInfo::LayerBacked:
            newDrawingArea = LayerBackedDrawingAreaProxy::create(this, m_page.get());
            break;
    }

    if (m_page->drawingArea())
        newDrawingArea->setSize(m_page->drawingArea()->size());

    m_page->drawingArea()->detachCompositingContext();
    m_page->setDrawingArea(newDrawingArea.release());
}

#endif // USE(ACCELERATED_COMPOSITING)

HWND WebView::nativeWindow()
{
    return m_window;
}

// WebCore::WindowMessageListener

void WebView::windowReceivedMessage(HWND, UINT message, WPARAM wParam, LPARAM)
{
    switch (message) {
        case WM_NCACTIVATE:
            updateActiveStateSoon();
            break;
        case WM_SETTINGCHANGE:
            // systemParameterChanged(wParam);
            break;
    }
}

} // namespace WebKit
