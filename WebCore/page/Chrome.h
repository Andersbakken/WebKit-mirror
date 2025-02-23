/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef Chrome_h
#define Chrome_h

#include "Cursor.h"
#include "FileChooser.h"
#include "FocusDirection.h"
#include "HostWindow.h"
#include "PopupMenu.h"
#include "SearchPopupMenu.h"
#include <wtf/Forward.h>
#include <wtf/RefPtr.h>

#if PLATFORM(MAC)
#ifndef __OBJC__
class NSView;
#endif
#endif

namespace WebCore {

    class ChromeClient;
    class ContextMenu;
    class FloatRect;
    class Frame;
    class Geolocation;
    class HitTestResult;
    class IntRect;
    class NavigationAction;
    class Node;
    class Page;
    class PopupMenuClient;
#if ENABLE(NOTIFICATIONS)
    class NotificationPresenter;
#endif

    struct FrameLoadRequest;
    struct ViewportArguments;
    struct WindowFeatures;
    
    class Chrome : public HostWindow {
    public:
        Chrome(Page*, ChromeClient*);
        ~Chrome();

        ChromeClient* client() { return m_client; }

        // HostWindow methods.

        virtual void invalidateWindow(const IntRect&, bool);
        virtual void invalidateContentsAndWindow(const IntRect&, bool);
        virtual void invalidateContentsForSlowScroll(const IntRect&, bool);
        virtual void scroll(const IntSize&, const IntRect&, const IntRect&);
#if ENABLE(TILED_BACKING_STORE)
        virtual void delegatedScrollRequested(const IntSize& scrollDelta);
#endif
        virtual IntPoint screenToWindow(const IntPoint&) const;
        virtual IntRect windowToScreen(const IntRect&) const;
        virtual PlatformPageClient platformPageClient() const;
        virtual void scrollbarsModeDidChange() const;
        virtual void setCursor(const Cursor&);

        void scrollRectIntoView(const IntRect&) const;

        void contentsSizeChanged(Frame*, const IntSize&) const;

        void setWindowRect(const FloatRect&) const;
        FloatRect windowRect() const;

        FloatRect pageRect() const;
        
        float scaleFactor();

        void focus() const;
        void unfocus() const;

        bool canTakeFocus(FocusDirection) const;
        void takeFocus(FocusDirection) const;

        void focusedNodeChanged(Node*) const;
        void focusedFrameChanged(Frame*) const;

        Page* createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&, const NavigationAction&) const;
        void show() const;

        bool canRunModal() const;
        bool canRunModalNow() const;
        void runModal() const;

        void setToolbarsVisible(bool) const;
        bool toolbarsVisible() const;
        
        void setStatusbarVisible(bool) const;
        bool statusbarVisible() const;
        
        void setScrollbarsVisible(bool) const;
        bool scrollbarsVisible() const;
        
        void setMenubarVisible(bool) const;
        bool menubarVisible() const;
        
        void setResizable(bool) const;

        bool canRunBeforeUnloadConfirmPanel();
        bool runBeforeUnloadConfirmPanel(const String& message, Frame* frame);

        void closeWindowSoon();

        void runJavaScriptAlert(Frame*, const String&);
        bool runJavaScriptConfirm(Frame*, const String&);
        bool runJavaScriptPrompt(Frame*, const String& message, const String& defaultValue, String& result);
        void setStatusbarText(Frame*, const String&);
        bool shouldInterruptJavaScript();

        IntRect windowResizerRect() const;

        void mouseDidMoveOverElement(const HitTestResult&, unsigned modifierFlags);

        void setToolTip(const HitTestResult&);

        void print(Frame*);

        // FIXME: Remove once all ports are using client-based geolocation. https://bugs.webkit.org/show_bug.cgi?id=40373
        // For client-based geolocation, these two methods have moved to GeolocationClient. https://bugs.webkit.org/show_bug.cgi?id=50061
        void requestGeolocationPermissionForFrame(Frame*, Geolocation*);
        void cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*);

        void runOpenPanel(Frame*, PassRefPtr<FileChooser>);
        void chooseIconForFiles(const Vector<String>&, FileChooser*);

        void dispatchViewportDataDidChange(const ViewportArguments&) const;

        bool requiresFullscreenForVideoPlayback();

#if PLATFORM(MAC)
        void focusNSView(NSView*);
#endif

#if ENABLE(NOTIFICATIONS)
        NotificationPresenter* notificationPresenter() const; 
#endif

        bool selectItemWritingDirectionIsNatural();
        PassRefPtr<PopupMenu> createPopupMenu(PopupMenuClient*) const;
        PassRefPtr<SearchPopupMenu> createSearchPopupMenu(PopupMenuClient*) const;

#if ENABLE(CONTEXT_MENUS)
        void showContextMenu();
#endif

    private:
        Page* m_page;
        ChromeClient* m_client;
    };
}

#endif // Chrome_h
