/*
 * Copyright (C) 2006, 2007, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef DOMWindow_h
#define DOMWindow_h

#include "KURL.h"
#include "MessagePort.h"
#include "SecurityOrigin.h"

namespace WebCore {

    class BarInfo;
    class Blob;
    class CSSRuleList;
    class CSSStyleDeclaration;
    class Console;
    class DOMApplicationCache;
    class DOMSelection;
    class Database;
    class DatabaseCallback;
    class Document;
    class Element;
    class ErrorCallback;
    class EventListener;
    class FileSystemCallback;
    class FloatRect;
    class History;
    class IDBFactory;
    class Location;
    class MediaQueryList;
    class Navigator;
    class Node;
    class NotificationCenter;
    class Performance;
    class PostMessageTimer;
    class ScheduledAction;
    class Screen;
    class SerializedScriptValue;
    class Storage;
    class StyleMedia;
    class WebKitPoint;

    struct WindowFeatures;

    typedef int ExceptionCode;

    enum SetLocationLocking { LockHistoryBasedOnGestureState, LockHistoryAndBackForwardList };

    class DOMWindow : public RefCounted<DOMWindow>, public EventTarget {
    public:
        static PassRefPtr<DOMWindow> create(Frame* frame) { return adoptRef(new DOMWindow(frame)); }
        virtual ~DOMWindow();

        virtual DOMWindow* toDOMWindow() { return this; }
        virtual ScriptExecutionContext* scriptExecutionContext() const;

        Frame* frame() const { return m_frame; }
        void disconnectFrame();

        void clear();

        PassRefPtr<MediaQueryList> matchMedia(const String&);

        void setSecurityOrigin(SecurityOrigin* securityOrigin) { m_securityOrigin = securityOrigin; }
        SecurityOrigin* securityOrigin() const { return m_securityOrigin.get(); }

        void setURL(const KURL& url) { m_url = url; }
        KURL url() const { return m_url; }

        unsigned pendingUnloadEventListeners() const;

        static bool dispatchAllPendingBeforeUnloadEvents();
        static void dispatchAllPendingUnloadEvents();

        static void adjustWindowRect(const FloatRect& screen, FloatRect& window, const FloatRect& pendingChanges);

        // FIXME: We can remove this function once V8 showModalDialog is changed to use DOMWindow.
        static void parseModalDialogFeatures(const String&, HashMap<String, String>&);

        bool allowPopUp(); // Call on first window, not target window.
        static bool allowPopUp(Frame* firstFrame);
        static bool canShowModalDialog(const Frame*);
        static bool canShowModalDialogNow(const Frame*);

        // DOM Level 0

        Screen* screen() const;
        History* history() const;
        BarInfo* locationbar() const;
        BarInfo* menubar() const;
        BarInfo* personalbar() const;
        BarInfo* scrollbars() const;
        BarInfo* statusbar() const;
        BarInfo* toolbar() const;
        Navigator* navigator() const;
        Navigator* clientInformation() const { return navigator(); }

        Location* location() const;
        void setLocation(const String& location, DOMWindow* activeWindow, DOMWindow* firstWindow,
            SetLocationLocking = LockHistoryBasedOnGestureState);

        DOMSelection* getSelection();

        Element* frameElement() const;

        void focus();
        void blur();
        void close();
        void print();
        void stop();

        PassRefPtr<DOMWindow> open(const String& urlString, const AtomicString& frameName, const String& windowFeaturesString,
            DOMWindow* activeWindow, DOMWindow* firstWindow);

        typedef void (*PrepareDialogFunction)(DOMWindow*, void* context);
        void showModalDialog(const String& urlString, const String& dialogFeaturesString,
            DOMWindow* activeWindow, DOMWindow* firstWindow, PrepareDialogFunction, void* functionContext);

        void alert(const String& message);
        bool confirm(const String& message);
        String prompt(const String& message, const String& defaultValue);
        String btoa(const String& stringToEncode, ExceptionCode&);
        String atob(const String& encodedString, ExceptionCode&);

        bool find(const String&, bool caseSensitive, bool backwards, bool wrap, bool wholeWord, bool searchInFrames, bool showDialog) const;

        bool offscreenBuffering() const;

        int outerHeight() const;
        int outerWidth() const;
        int innerHeight() const;
        int innerWidth() const;
        int screenX() const;
        int screenY() const;
        int screenLeft() const { return screenX(); }
        int screenTop() const { return screenY(); }
        int scrollX() const;
        int scrollY() const;
        int pageXOffset() const { return scrollX(); }
        int pageYOffset() const { return scrollY(); }

        bool closed() const;

        unsigned length() const;

        String name() const;
        void setName(const String&);

        String status() const;
        void setStatus(const String&);
        String defaultStatus() const;
        void setDefaultStatus(const String&);

        // This attribute is an alias of defaultStatus and is necessary for legacy uses.
        String defaultstatus() const { return defaultStatus(); }
        void setDefaultstatus(const String& status) { setDefaultStatus(status); }

        // Self-referential attributes

        DOMWindow* self() const;
        DOMWindow* window() const { return self(); }
        DOMWindow* frames() const { return self(); }

        DOMWindow* opener() const;
        DOMWindow* parent() const;
        DOMWindow* top() const;

        // DOM Level 2 AbstractView Interface

        Document* document() const;

        // CSSOM View Module

        PassRefPtr<StyleMedia> styleMedia() const;

        // DOM Level 2 Style Interface

        PassRefPtr<CSSStyleDeclaration> getComputedStyle(Element*, const String& pseudoElt) const;

        // WebKit extensions

        PassRefPtr<CSSRuleList> getMatchedCSSRules(Element*, const String& pseudoElt, bool authorOnly = true) const;
        double devicePixelRatio() const;

        PassRefPtr<WebKitPoint> webkitConvertPointFromPageToNode(Node*, const WebKitPoint*) const;
        PassRefPtr<WebKitPoint> webkitConvertPointFromNodeToPage(Node*, const WebKitPoint*) const;        

        Console* console() const;

        void printErrorMessage(const String&);
        String crossDomainAccessErrorMessage(DOMWindow* activeWindow);

        void pageDestroyed();

        void postMessage(PassRefPtr<SerializedScriptValue> message, const MessagePortArray*, const String& targetOrigin, DOMWindow* source, ExceptionCode&);
        // FIXME: remove this when we update the ObjC bindings (bug #28774).
        void postMessage(PassRefPtr<SerializedScriptValue> message, MessagePort*, const String& targetOrigin, DOMWindow* source, ExceptionCode&);
        void postMessageTimerFired(PostMessageTimer*);

        void scrollBy(int x, int y) const;
        void scrollTo(int x, int y) const;
        void scroll(int x, int y) const { scrollTo(x, y); }

        void moveBy(float x, float y) const;
        void moveTo(float x, float y) const;

        void resizeBy(float x, float y) const;
        void resizeTo(float width, float height) const;

        // Timers
        int setTimeout(PassOwnPtr<ScheduledAction>, int timeout, ExceptionCode&);
        void clearTimeout(int timeoutId);
        int setInterval(PassOwnPtr<ScheduledAction>, int timeout, ExceptionCode&);
        void clearInterval(int timeoutId);

        // Events
        // EventTarget API
        virtual bool addEventListener(const AtomicString& eventType, PassRefPtr<EventListener>, bool useCapture);
        virtual bool removeEventListener(const AtomicString& eventType, EventListener*, bool useCapture);
        virtual void removeAllEventListeners();

        using EventTarget::dispatchEvent;
        bool dispatchEvent(PassRefPtr<Event> prpEvent, PassRefPtr<EventTarget> prpTarget);
        void dispatchLoadEvent();
        void dispatchTimedEvent(PassRefPtr<Event> event, Document* target, double* startTime, double* endTime);

        DEFINE_ATTRIBUTE_EVENT_LISTENER(abort);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(beforeunload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(blur);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplay);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplaythrough);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(change);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(click);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(contextmenu);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dblclick);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drag);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragenter);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragleave);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drop);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(durationchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(emptied);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ended);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(focus);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(hashchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(input);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(invalid);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keydown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keypress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keyup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(load);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadeddata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadedmetadata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(message);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousedown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousemove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseout);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousewheel);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(offline);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(online);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pagehide);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pageshow);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pause);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(play);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(playing);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(popstate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(progress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ratechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(reset);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(resize);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(scroll);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(search);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeked);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeking);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(select);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(stalled);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(storage);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(submit);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(suspend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(timeupdate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(unload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(volumechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(waiting);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitbeginfullscreen);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitendfullscreen);

        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationstart, webkitAnimationStart);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationiteration, webkitAnimationIteration);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationend, webkitAnimationEnd);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkittransitionend, webkitTransitionEnd);

        void captureEvents();
        void releaseEvents();

        void finishedLoading();

        // These functions are used for GC marking. See JSDOMWindow::markChildren(MarkStack&) in JSDOMWindowCustom.cpp.
        Screen* optionalScreen() const { return m_screen.get(); }
        DOMSelection* optionalSelection() const { return m_selection.get(); }
        History* optionalHistory() const { return m_history.get(); }
        BarInfo* optionalLocationbar() const { return m_locationbar.get(); }
        BarInfo* optionalMenubar() const { return m_menubar.get(); }
        BarInfo* optionalPersonalbar() const { return m_personalbar.get(); }
        BarInfo* optionalScrollbars() const { return m_scrollbars.get(); }
        BarInfo* optionalStatusbar() const { return m_statusbar.get(); }
        BarInfo* optionalToolbar() const { return m_toolbar.get(); }
        Console* optionalConsole() const { return m_console.get(); }
        Navigator* optionalNavigator() const { return m_navigator.get(); }
        Location* optionalLocation() const { return m_location.get(); }
        StyleMedia* optionalMedia() const { return m_media.get(); }

        using RefCounted<DOMWindow>::ref;
        using RefCounted<DOMWindow>::deref;

#if ENABLE(BLOB)
        String createObjectURL(Blob*);
        void revokeObjectURL(const String&);
#endif

#if ENABLE(DATABASE)
        // HTML 5 client-side database
        PassRefPtr<Database> openDatabase(const String& name, const String& version, const String& displayName, unsigned long estimatedSize, PassRefPtr<DatabaseCallback> creationCallback, ExceptionCode&);
#endif

#if ENABLE(DEVICE_ORIENTATION)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(devicemotion);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(deviceorientation);
#endif

#if ENABLE(DOM_STORAGE)
        // HTML 5 key/value storage
        Storage* sessionStorage(ExceptionCode&) const;
        Storage* localStorage(ExceptionCode&) const;
        Storage* optionalSessionStorage() const { return m_sessionStorage.get(); }
        Storage* optionalLocalStorage() const { return m_localStorage.get(); }
#endif

#if ENABLE(FILE_SYSTEM)
        // They are placed here and in all capital letters so they can be checked against the constants in the
        // IDL at compile time.
        enum FileSystemType {
            TEMPORARY,
            PERSISTENT,
        };
        void requestFileSystem(int type, long long size, PassRefPtr<FileSystemCallback>, PassRefPtr<ErrorCallback>);
#endif

#if ENABLE(INDEXED_DATABASE)
        IDBFactory* webkitIndexedDB() const;
#endif

#if ENABLE(NOTIFICATIONS)
        NotificationCenter* webkitNotifications() const;
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
        DOMApplicationCache* applicationCache() const;
        DOMApplicationCache* optionalApplicationCache() const { return m_applicationCache.get(); }
#endif

#if ENABLE(ORIENTATION_EVENTS)
        // This is the interface orientation in degrees. Some examples are:
        //  0 is straight up; -90 is when the device is rotated 90 clockwise;
        //  90 is when rotated counter clockwise.
        int orientation() const;

        DEFINE_ATTRIBUTE_EVENT_LISTENER(orientationchange);
#endif

#if ENABLE(TOUCH_EVENTS)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchmove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchcancel);
#endif

#if ENABLE(WEB_TIMING)
        Performance* webkitPerformance() const;
        Performance* optionalWebkitPerformance() const { return m_performance.get(); }
#endif

    private:
        DOMWindow(Frame*);

        virtual void refEventTarget() { ref(); }
        virtual void derefEventTarget() { deref(); }
        virtual EventTargetData* eventTargetData();
        virtual EventTargetData* ensureEventTargetData();

        static Frame* createWindow(const String& urlString, const AtomicString& frameName, const WindowFeatures&,
            DOMWindow* activeWindow, Frame* firstFrame, Frame* openerFrame,
            PrepareDialogFunction = 0, void* functionContext = 0);
        bool isInsecureScriptAccess(DOMWindow* activeWindow, const String& urlString);

        RefPtr<SecurityOrigin> m_securityOrigin;
        KURL m_url;

        bool m_shouldPrintWhenFinishedLoading;
        Frame* m_frame;
        mutable RefPtr<Screen> m_screen;
        mutable RefPtr<DOMSelection> m_selection;
        mutable RefPtr<History> m_history;
        mutable RefPtr<BarInfo> m_locationbar;
        mutable RefPtr<BarInfo> m_menubar;
        mutable RefPtr<BarInfo> m_personalbar;
        mutable RefPtr<BarInfo> m_scrollbars;
        mutable RefPtr<BarInfo> m_statusbar;
        mutable RefPtr<BarInfo> m_toolbar;
        mutable RefPtr<Console> m_console;
        mutable RefPtr<Navigator> m_navigator;
        mutable RefPtr<Location> m_location;
        mutable RefPtr<StyleMedia> m_media;

        EventTargetData m_eventTargetData;

        String m_status;
        String m_defaultStatus;

#if ENABLE(DOM_STORAGE)
        mutable RefPtr<Storage> m_sessionStorage;
        mutable RefPtr<Storage> m_localStorage;
#endif

#if ENABLE(INDEXED_DATABASE)
        mutable RefPtr<IDBFactory> m_idbFactory;
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
        mutable RefPtr<DOMApplicationCache> m_applicationCache;
#endif

#if ENABLE(NOTIFICATIONS)
        mutable RefPtr<NotificationCenter> m_notifications;
#endif

#if ENABLE(WEB_TIMING)
        mutable RefPtr<Performance> m_performance;
#endif
    };

    inline String DOMWindow::status() const
    {
        return m_status;
    }

    inline String DOMWindow::defaultStatus() const
    {
        return m_defaultStatus;
    } 

} // namespace WebCore

#endif // DOMWindow_h
