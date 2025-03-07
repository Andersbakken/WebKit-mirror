/*
 * Copyright (C) 2006, 2007, 2009, 2010 Apple Inc. All rights reserved.
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

#ifndef EventHandler_h
#define EventHandler_h

#include "DragActions.h"
#include "FocusDirection.h"
#include "HitTestRequest.h"
#include "PlatformMouseEvent.h"
#include "ScrollTypes.h"
#include "Timer.h"
#include <wtf/Forward.h>
#include <wtf/RefPtr.h>

#if PLATFORM(MAC) && !defined(__OBJC__)
class NSView;
#endif

#if ENABLE(TOUCH_EVENTS)
#include <wtf/HashMap.h>
#endif

namespace WebCore {

class Clipboard;
class Cursor;
class Event;
class EventTarget;
class FloatPoint;
class FloatQuad;
class Frame;
class HitTestRequest;
class HitTestResult;
class HTMLFrameSetElement;
class KeyboardEvent;
class MouseEventWithHitTestResults;
class Node;
class PlatformKeyboardEvent;
class PlatformTouchEvent;
class PlatformWheelEvent;
class RenderLayer;
class RenderObject;
class RenderWidget;
class Scrollbar;
class SVGElementInstance;
class TextEvent;
class TouchEvent;
class WheelEvent;
class Widget;
    
#if ENABLE(DRAG_SUPPORT)
extern const int LinkDragHysteresis;
extern const int ImageDragHysteresis;
extern const int TextDragHysteresis;
extern const int GeneralDragHysteresis;
#endif // ENABLE(DRAG_SUPPORT)

enum HitTestScrollbars { ShouldHitTestScrollbars, DontHitTestScrollbars };

class EventHandler : public Noncopyable {
public:
    EventHandler(Frame*);
    ~EventHandler();

    void clear();

#if ENABLE(DRAG_SUPPORT)
    void updateSelectionForMouseDrag();
#endif

    Node* mousePressNode() const;
    void setMousePressNode(PassRefPtr<Node>);

    void startPanScrolling(RenderObject*);
    bool panScrollInProgress() { return m_panScrollInProgress; }
    void setPanScrollInProgress(bool inProgress) { m_panScrollInProgress = inProgress; }

    void stopAutoscrollTimer(bool rendererIsBeingDestroyed = false);
    RenderObject* autoscrollRenderer() const;
    void updateAutoscrollRenderer();

    void dispatchFakeMouseMoveEventSoonInQuad(const FloatQuad&);

    HitTestResult hitTestResultAtPoint(const IntPoint&, bool allowShadowContent, bool ignoreClipping = false,
                                       HitTestScrollbars scrollbars = DontHitTestScrollbars,
                                       HitTestRequest::HitTestRequestType hitType = HitTestRequest::ReadOnly | HitTestRequest::Active,
                                       const IntSize& padding = IntSize());

    bool mousePressed() const { return m_mousePressed; }
    void setMousePressed(bool pressed) { m_mousePressed = pressed; }

    void setCapturingMouseEventsNode(PassRefPtr<Node>); // A caller is responsible for resetting capturing node to 0.

#if ENABLE(DRAG_SUPPORT)
    bool updateDragAndDrop(const PlatformMouseEvent&, Clipboard*);
    void cancelDragAndDrop(const PlatformMouseEvent&, Clipboard*);
    bool performDragAndDrop(const PlatformMouseEvent&, Clipboard*);
#endif

    void scheduleHoverStateUpdate();

    void setResizingFrameSet(HTMLFrameSetElement*);

    void resizeLayerDestroyed();

    IntPoint currentMousePosition() const;

    void setIgnoreWheelEvents(bool);

    static Frame* subframeForTargetNode(Node*);

    bool scrollOverflow(ScrollDirection, ScrollGranularity, Node* startingNode = 0);
    bool logicalScrollOverflow(ScrollLogicalDirection, ScrollGranularity, Node* startingNode = 0);

    bool scrollRecursively(ScrollDirection, ScrollGranularity, Node* startingNode = 0);
    bool logicalScrollRecursively(ScrollLogicalDirection, ScrollGranularity, Node* startingNode = 0);

#if ENABLE(DRAG_SUPPORT)
    bool shouldDragAutoNode(Node*, const IntPoint&) const; // -webkit-user-drag == auto
#endif

    bool shouldTurnVerticalTicksIntoHorizontal(const HitTestResult&) const;

    bool tabsToLinks(KeyboardEvent*) const;
    bool tabsToAllControls(KeyboardEvent*) const;

    bool mouseDownMayStartSelect() const { return m_mouseDownMayStartSelect; }

    bool mouseMoved(const PlatformMouseEvent&);

    void lostMouseCapture();

    bool handleMousePressEvent(const PlatformMouseEvent&);
    bool handleMouseMoveEvent(const PlatformMouseEvent&, HitTestResult* hoveredNode = 0);
    bool handleMouseReleaseEvent(const PlatformMouseEvent&);
    bool handleWheelEvent(PlatformWheelEvent&);
    void defaultWheelEventHandler(Node*, WheelEvent*);

#if ENABLE(CONTEXT_MENUS)
    bool sendContextMenuEvent(const PlatformMouseEvent&);
    bool sendContextMenuEventForKey();
#endif

    void setMouseDownMayStartAutoscroll() { m_mouseDownMayStartAutoscroll = true; }

    bool needsKeyboardEventDisambiguationQuirks() const;

    static unsigned accessKeyModifiers();
    bool handleAccessKey(const PlatformKeyboardEvent&);
    bool keyEvent(const PlatformKeyboardEvent&);
    void defaultKeyboardEventHandler(KeyboardEvent*);

    bool handleTextInputEvent(const String& text, Event* underlyingEvent = 0,
        bool isLineBreak = false, bool isBackTab = false);
    void defaultTextInputEventHandler(TextEvent*);

#if ENABLE(DRAG_SUPPORT)
    bool eventMayStartDrag(const PlatformMouseEvent&) const;
    
    void dragSourceEndedAt(const PlatformMouseEvent&, DragOperation);
#endif

    void focusDocumentView();

    void capsLockStateMayHaveChanged();
    
    void sendResizeEvent();
    void sendScrollEvent();
    
#if PLATFORM(MAC) && defined(__OBJC__)
    PassRefPtr<KeyboardEvent> currentKeyboardEvent() const;

    void mouseDown(NSEvent *);
    void mouseDragged(NSEvent *);
    void mouseUp(NSEvent *);
    void mouseMoved(NSEvent *);
    bool keyEvent(NSEvent *);
    bool wheelEvent(NSEvent *);

#if ENABLE(CONTEXT_MENUS)
    bool sendContextMenuEvent(NSEvent *);
#endif
    bool eventMayStartDrag(NSEvent *);

    void sendFakeEventsAfterWidgetTracking(NSEvent *initiatingEvent);

    void setActivationEventNumber(int num) { m_activationEventNumber = num; }

    static NSEvent *currentNSEvent();
#endif

#if ENABLE(TOUCH_EVENTS)
    bool handleTouchEvent(const PlatformTouchEvent&);
#endif

private:
#if ENABLE(DRAG_SUPPORT)
    enum DragAndDropHandleType {
        UpdateDragAndDrop,
        CancelDragAndDrop,
        PerformDragAndDrop
    };

    struct EventHandlerDragState : Noncopyable {
        RefPtr<Node> m_dragSrc; // element that may be a drag source, for the current mouse gesture
        bool m_dragSrcIsLink;
        bool m_dragSrcIsImage;
        bool m_dragSrcInSelection;
        bool m_dragSrcMayBeDHTML;
        bool m_dragSrcMayBeUA; // Are DHTML and/or the UserAgent allowed to drag out?
        bool m_dragSrcIsDHTML;
        RefPtr<Clipboard> m_dragClipboard; // used on only the source side of dragging
    };
    static EventHandlerDragState& dragState();
    static const double TextDragDelay;

    bool canHandleDragAndDropForTarget(DragAndDropHandleType, Node* target, const PlatformMouseEvent&, Clipboard*, bool* accepted = 0);
    
    PassRefPtr<Clipboard> createDraggingClipboard() const;
#endif // ENABLE(DRAG_SUPPORT)
    
    bool eventActivatedView(const PlatformMouseEvent&) const;
    void selectClosestWordFromMouseEvent(const MouseEventWithHitTestResults&);
    void selectClosestWordOrLinkFromMouseEvent(const MouseEventWithHitTestResults&);

    bool handleMouseDoubleClickEvent(const PlatformMouseEvent&);

    bool handleMousePressEvent(const MouseEventWithHitTestResults&);
    bool handleMousePressEventSingleClick(const MouseEventWithHitTestResults&);
    bool handleMousePressEventDoubleClick(const MouseEventWithHitTestResults&);
    bool handleMousePressEventTripleClick(const MouseEventWithHitTestResults&);
#if ENABLE(DRAG_SUPPORT)
    bool handleMouseDraggedEvent(const MouseEventWithHitTestResults&);
#endif
    bool handleMouseReleaseEvent(const MouseEventWithHitTestResults&);

    void handleKeyboardSelectionMovement(KeyboardEvent*);
    
    Cursor selectCursor(const MouseEventWithHitTestResults&, Scrollbar*);
#if ENABLE(PAN_SCROLLING)
    void updatePanScrollState();
#endif

    void hoverTimerFired(Timer<EventHandler>*);

    static bool canMouseDownStartSelect(Node*);
#if ENABLE(DRAG_SUPPORT)
    static bool canMouseDragExtendSelect(Node*);
#endif

    void handleAutoscroll(RenderObject*);
    void startAutoscrollTimer();
    void setAutoscrollRenderer(RenderObject*);
    void autoscrollTimerFired(Timer<EventHandler>*);

    void fakeMouseMoveEventTimerFired(Timer<EventHandler>*);
    void cancelFakeMouseMoveEvent();

    void invalidateClick();

    Node* nodeUnderMouse() const;
    
    void updateMouseEventTargetNode(Node*, const PlatformMouseEvent&, bool fireMouseOverOut);
    void fireMouseOverOut(bool fireMouseOver = true, bool fireMouseOut = true, bool updateLastNodeUnderMouse = true);
    
    MouseEventWithHitTestResults prepareMouseEvent(const HitTestRequest&, const PlatformMouseEvent&);

    bool dispatchMouseEvent(const AtomicString& eventType, Node* target, bool cancelable, int clickCount, const PlatformMouseEvent&, bool setUnder);
#if ENABLE(DRAG_SUPPORT)
    bool dispatchDragEvent(const AtomicString& eventType, Node* target, const PlatformMouseEvent&, Clipboard*);

    void freeClipboard();

    bool handleDrag(const MouseEventWithHitTestResults&);
#endif
    bool handleMouseUp(const MouseEventWithHitTestResults&);
#if ENABLE(DRAG_SUPPORT)
    void clearDragState();

    bool dispatchDragSrcEvent(const AtomicString& eventType, const PlatformMouseEvent&);

    bool dragHysteresisExceeded(const FloatPoint&) const;
    bool dragHysteresisExceeded(const IntPoint&) const;
#endif // ENABLE(DRAG_SUPPORT)

    bool passMousePressEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);
    bool passMouseMoveEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe, HitTestResult* hoveredNode = 0);
    bool passMouseReleaseEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe);

    bool passSubframeEventToSubframe(MouseEventWithHitTestResults&, Frame* subframe, HitTestResult* hoveredNode = 0);

    bool passMousePressEventToScrollbar(MouseEventWithHitTestResults&, Scrollbar*);

    bool passWidgetMouseDownEventToWidget(const MouseEventWithHitTestResults&);
    bool passWidgetMouseDownEventToWidget(RenderWidget*);

    bool passMouseDownEventToWidget(Widget*);
    bool passWheelEventToWidget(PlatformWheelEvent&, Widget*);

    void defaultSpaceEventHandler(KeyboardEvent*);
    void defaultTabEventHandler(KeyboardEvent*);
    void defaultArrowEventHandler(FocusDirection, KeyboardEvent*);

#if ENABLE(DRAG_SUPPORT)
    void allowDHTMLDrag(bool& flagDHTML, bool& flagUA) const;
#endif

    // The following are called at the beginning of handleMouseUp and handleDrag.  
    // If they return true it indicates that they have consumed the event.
    bool eventLoopHandleMouseUp(const MouseEventWithHitTestResults&);
#if ENABLE(DRAG_SUPPORT)
    bool eventLoopHandleMouseDragged(const MouseEventWithHitTestResults&);
#endif

    bool invertSenseOfTabsToLinks(KeyboardEvent*) const;

#if ENABLE(DRAG_SUPPORT)
    void updateSelectionForMouseDrag(Node* targetNode, const IntPoint& localPoint);
#endif

    void updateLastScrollbarUnderMouse(Scrollbar*, bool);
    
    void setFrameWasScrolledByUser();

    FocusDirection focusDirectionForKey(const AtomicString&) const;

    bool capturesDragging() const { return m_capturesDragging; }

#if PLATFORM(MAC) && defined(__OBJC__)
    NSView *mouseDownViewIfStillGood();

    PlatformMouseEvent currentPlatformMouseEvent() const;
#endif

    Frame* m_frame;

    bool m_mousePressed;
    bool m_capturesDragging;
    RefPtr<Node> m_mousePressNode;

    bool m_mouseDownMayStartSelect;
#if ENABLE(DRAG_SUPPORT)
    bool m_mouseDownMayStartDrag;
#endif
    bool m_mouseDownWasSingleClickInSelection;
    bool m_beganSelectingText;

#if ENABLE(DRAG_SUPPORT)
    IntPoint m_dragStartPos;
#endif

    IntPoint m_panScrollStartPos;
    bool m_panScrollInProgress;

    bool m_panScrollButtonPressed;
    bool m_springLoadedPanScrollInProgress;

    Timer<EventHandler> m_hoverTimer;
    
    Timer<EventHandler> m_autoscrollTimer;
    RenderObject* m_autoscrollRenderer;
    bool m_autoscrollInProgress;
    bool m_mouseDownMayStartAutoscroll;
    bool m_mouseDownWasInSubframe;

    Timer<EventHandler> m_fakeMouseMoveEventTimer;

#if ENABLE(SVG)
    bool m_svgPan;
    RefPtr<SVGElementInstance> m_instanceUnderMouse;
    RefPtr<SVGElementInstance> m_lastInstanceUnderMouse;
#endif

    RenderLayer* m_resizeLayer;

    RefPtr<Node> m_capturingMouseEventsNode;
    bool m_eventHandlerWillResetCapturingMouseEventsNode;
    
    RefPtr<Node> m_nodeUnderMouse;
    RefPtr<Node> m_lastNodeUnderMouse;
    RefPtr<Frame> m_lastMouseMoveEventSubframe;
    RefPtr<Scrollbar> m_lastScrollbarUnderMouse;

    int m_clickCount;
    RefPtr<Node> m_clickNode;

#if ENABLE(DRAG_SUPPORT)
    RefPtr<Node> m_dragTarget;
    bool m_shouldOnlyFireDragOverEvent;
#endif
    
    RefPtr<HTMLFrameSetElement> m_frameSetBeingResized;

    IntSize m_offsetFromResizeCorner;   // in the coords of m_resizeLayer
    
    IntPoint m_currentMousePosition;
    IntPoint m_mouseDownPos; // in our view's coords
    double m_mouseDownTimestamp;
    PlatformMouseEvent m_mouseDown;

    bool m_useLatchedWheelEventNode;
    RefPtr<Node> m_latchedWheelEventNode;
    bool m_widgetIsLatched;

    RefPtr<Node> m_previousWheelScrolledNode;

#if PLATFORM(MAC)
    NSView *m_mouseDownView;
    bool m_sendingEventToSubview;
    int m_activationEventNumber;
#endif
#if ENABLE(TOUCH_EVENTS)
    typedef HashMap<int, RefPtr<EventTarget> > TouchTargetMap;
    TouchTargetMap m_originatingTouchPointTargets;
    bool m_touchPressed;
#endif
};

} // namespace WebCore

#endif // EventHandler_h
