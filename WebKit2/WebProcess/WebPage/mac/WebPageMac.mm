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

#include "WebPage.h"

#include "PluginView.h"
#include "WebCoreArgumentCoders.h"
#include "WebEvent.h"
#include "WebPageProxyMessages.h"
#include "WebProcess.h"
#include <WebCore/FocusController.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameView.h>
#include <WebCore/HitTestResult.h>
#include <WebCore/KeyboardEvent.h>
#include <WebCore/Page.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include <WebCore/ScrollView.h>
#include <WebCore/TextIterator.h>
#include <WebCore/WindowsKeyboardCodes.h>

using namespace WebCore;

namespace WebKit {

void WebPage::platformInitialize()
{
    m_page->addSchedulePair(SchedulePair::create([NSRunLoop currentRunLoop], kCFRunLoopCommonModes));
}

void WebPage::platformPreferencesDidChange(const WebPreferencesStore&)
{
}

// FIXME: need to add support for input methods
    
bool WebPage::interceptEditingKeyboardEvent(KeyboardEvent* evt, bool shouldSaveCommand)
{
    Node* node = evt->target()->toNode();
    ASSERT(node);
    Frame* frame = node->document()->frame();
    ASSERT(frame);
    
    const PlatformKeyboardEvent* keyEvent = evt->keyEvent();
    if (!keyEvent)
        return false;
    const Vector<KeypressCommand>& commands = evt->keypressCommands();
    bool hasKeypressCommand = !commands.isEmpty();
    
    bool eventWasHandled = false;
    
    if (shouldSaveCommand || !hasKeypressCommand) {
        Vector<KeypressCommand> commandsList;  
        Vector<CompositionUnderline> underlines;
        unsigned start;
        unsigned end;
        if (!WebProcess::shared().connection()->sendSync(Messages::WebPageProxy::InterpretKeyEvent(keyEvent->type()), 
                                                         Messages::WebPageProxy::InterpretKeyEvent::Reply(commandsList, start, end, underlines),
                                                         m_pageID, CoreIPC::Connection::NoTimeout))
            return false;
        if (commandsList.isEmpty())
            return eventWasHandled;
        
        if (commandsList[0].commandName == "setMarkedText") {
            frame->editor()->setComposition(commandsList[0].text, underlines, start, end);
            eventWasHandled = true;
        } else if (commandsList[0].commandName == "insertText" && frame->editor()->hasComposition()) {
            frame->editor()->confirmComposition(commandsList[0].text);
            eventWasHandled = true;
        } else if (commandsList[0].commandName == "unmarkText") {
            frame->editor()->confirmComposition();
            eventWasHandled = true;
        } else {
            for (size_t i = 0; i < commandsList.size(); i++)
                evt->keypressCommands().append(commandsList[i]);
        }
    } else {
        size_t size = commands.size();
        // Are there commands that would just cause text insertion if executed via Editor?
        // WebKit doesn't have enough information about mode to decide how they should be treated, so we leave it upon WebCore
        // to either handle them immediately (e.g. Tab that changes focus) or let a keypress event be generated
        // (e.g. Tab that inserts a Tab character, or Enter).
        bool haveTextInsertionCommands = false;
        for (size_t i = 0; i < size; ++i) {
            if (frame->editor()->command(commands[i].commandName).isTextInsertion())
                haveTextInsertionCommands = true;
        }
        if (!haveTextInsertionCommands || keyEvent->type() == PlatformKeyboardEvent::Char) {
            for (size_t i = 0; i < size; ++i) {
                if (commands[i].commandName == "insertText") {
                    // Don't insert null or control characters as they can result in unexpected behaviour
                    if (evt->charCode() < ' ')
                        return false;
                    eventWasHandled = frame->editor()->insertText(commands[i].text, evt);
                } else
                    if (frame->editor()->command(commands[i].commandName).isSupported())
                        eventWasHandled = frame->editor()->command(commands[i].commandName).execute(evt);
            }
        }
    }
    return eventWasHandled;
}

void WebPage::sendComplexTextInputToPlugin(uint64_t pluginComplexTextInputIdentifier, const String& textInput)
{
    for (HashSet<PluginView*>::const_iterator it = m_pluginViews.begin(), end = m_pluginViews.end(); it != end; ++it) {
        if ((*it)->sendComplexTextInput(pluginComplexTextInputIdentifier, textInput))
            break;
    }
}

void WebPage::getMarkedRange(uint64_t& location, uint64_t& length)
{
    location = NSNotFound;
    length = 0;
    Frame* frame = m_page->focusController()->focusedOrMainFrame();
    if (!frame)
        return;
    
    getLocationAndLengthFromRange(frame->editor()->compositionRange().get(), location, length);
}

static Range *characterRangeAtPoint(Frame* frame, const IntPoint point)
{
    VisiblePosition position = frame->visiblePositionForPoint(point);
    if (position.isNull())
        return NULL;
    
    VisiblePosition previous = position.previous();
    if (previous.isNotNull()) {
        Range *previousCharacterRange = makeRange(previous, position).get();
        NSRect rect = frame->editor()->firstRectForRange(previousCharacterRange);
        if (NSPointInRect(point, rect))
            return previousCharacterRange;
    }
    
    VisiblePosition next = position.next();
    if (next.isNotNull()) {
        Range *nextCharacterRange = makeRange(position, next).get();
        NSRect rect = frame->editor()->firstRectForRange(nextCharacterRange);
        if (NSPointInRect(point, rect))
            return nextCharacterRange;
    }
    
    return NULL;
}
    
void WebPage::characterIndexForPoint(IntPoint point, uint64_t& index)
{
    index = NSNotFound;
    Frame* frame = m_page->mainFrame();
    if (!frame)
        return;

    HitTestResult result = frame->eventHandler()->hitTestResultAtPoint(point, false);
    frame = result.innerNonSharedNode() ? result.innerNonSharedNode()->document()->frame() : m_page->focusController()->focusedOrMainFrame();
    
    Range *range = characterRangeAtPoint(frame, result.point());
    if (!range)
        return;

    uint64_t length;
    getLocationAndLengthFromRange(range, index, length);
}

static PassRefPtr<Range> convertToRange(Frame* frame, NSRange nsrange)
{
    if (nsrange.location > INT_MAX)
        return 0;
    if (nsrange.length > INT_MAX || nsrange.location + nsrange.length > INT_MAX)
        nsrange.length = INT_MAX - nsrange.location;
        
    // our critical assumption is that we are only called by input methods that
    // concentrate on a given area containing the selection
    // We have to do this because of text fields and textareas. The DOM for those is not
    // directly in the document DOM, so serialization is problematic. Our solution is
    // to use the root editable element of the selection start as the positional base.
    // That fits with AppKit's idea of an input context.
    Element* selectionRoot = frame->selection()->rootEditableElement();
    Element* scope = selectionRoot ? selectionRoot : frame->document()->documentElement();
    return TextIterator::rangeFromLocationAndLength(scope, nsrange.location, nsrange.length);
}
    
void WebPage::firstRectForCharacterRange(uint64_t location, uint64_t length, WebCore::IntRect& resultRect)
{
    Frame* frame = m_page->focusController()->focusedOrMainFrame();
    resultRect.setLocation(IntPoint(0, 0));
    resultRect.setSize(IntSize(0, 0));
    
    RefPtr<Range> range = convertToRange(frame, NSMakeRange(location, length));
    if (range) {
        ASSERT(range->startContainer());
        ASSERT(range->endContainer());
    }
     
    IntRect rect = frame->editor()->firstRectForRange(range.get());
    resultRect = frame->view()->contentsToWindow(rect);
}

static inline void scroll(Page* page, ScrollDirection direction, ScrollGranularity granularity)
{
    page->focusController()->focusedOrMainFrame()->eventHandler()->scrollRecursively(direction, granularity);
}

static inline void logicalScroll(Page* page, ScrollLogicalDirection direction, ScrollGranularity granularity)
{
    page->focusController()->focusedOrMainFrame()->eventHandler()->logicalScrollRecursively(direction, granularity);
}

bool WebPage::performDefaultBehaviorForKeyEvent(const WebKeyboardEvent& keyboardEvent)
{
    if (keyboardEvent.type() != WebEvent::KeyDown)
        return false;

    // FIXME: This should be in WebCore.

    switch (keyboardEvent.windowsVirtualKeyCode()) {
    case VK_BACK:
        if (keyboardEvent.shiftKey())
            m_page->goForward();
        else
            m_page->goBack();
        break;
    case VK_SPACE:
        if (keyboardEvent.shiftKey())
            logicalScroll(m_page.get(), ScrollBlockDirectionBackward, ScrollByPage);
        else
            logicalScroll(m_page.get(), ScrollBlockDirectionForward, ScrollByPage);
        break;
    case VK_PRIOR:
        logicalScroll(m_page.get(), ScrollBlockDirectionBackward, ScrollByPage);
        break;
    case VK_NEXT:
        logicalScroll(m_page.get(), ScrollBlockDirectionForward, ScrollByPage);
        break;
    case VK_HOME:
        logicalScroll(m_page.get(), ScrollBlockDirectionBackward, ScrollByDocument);
        break;
    case VK_END:
        logicalScroll(m_page.get(), ScrollBlockDirectionForward, ScrollByDocument);
        break;
    case VK_UP:
        if (keyboardEvent.shiftKey())
            return false;
        if (keyboardEvent.metaKey()) {
            scroll(m_page.get(), ScrollUp, ScrollByDocument);
            scroll(m_page.get(), ScrollLeft, ScrollByDocument);
        } else if (keyboardEvent.altKey() || keyboardEvent.controlKey())
            scroll(m_page.get(), ScrollUp, ScrollByPage);
        else
            scroll(m_page.get(), ScrollUp, ScrollByLine);
        break;
    case VK_DOWN:
        if (keyboardEvent.shiftKey())
            return false;
        if (keyboardEvent.metaKey()) {
            scroll(m_page.get(), ScrollDown, ScrollByDocument);
            scroll(m_page.get(), ScrollLeft, ScrollByDocument);
        } else if (keyboardEvent.altKey() || keyboardEvent.controlKey())
            scroll(m_page.get(), ScrollDown, ScrollByPage);
        else
            scroll(m_page.get(), ScrollDown, ScrollByLine);
        break;
    case VK_LEFT:
        if (keyboardEvent.shiftKey())
            return false;
        if (keyboardEvent.metaKey())
            m_page->goBack();
        else {
            if (keyboardEvent.altKey()  | keyboardEvent.controlKey())
                scroll(m_page.get(), ScrollLeft, ScrollByPage);
            else
                scroll(m_page.get(), ScrollLeft, ScrollByLine);
        }
        break;
    case VK_RIGHT:
        if (keyboardEvent.shiftKey())
            return false;
        if (keyboardEvent.metaKey())
            m_page->goForward();
        else {
            if (keyboardEvent.altKey() || keyboardEvent.controlKey())
                scroll(m_page.get(), ScrollRight, ScrollByPage);
            else
                scroll(m_page.get(), ScrollRight, ScrollByLine);
        }
        break;
    default:
        return false;
    }

    return true;
}

bool WebPage::platformHasLocalDataForURL(const WebCore::KURL& url)
{
    NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL:url];
    [request setValue:(NSString*)userAgent() forHTTPHeaderField:@"User-Agent"];
    NSCachedURLResponse *cachedResponse = [[NSURLCache sharedURLCache] cachedResponseForRequest:request];
    [request release];
    
    return cachedResponse;
}

bool WebPage::canHandleRequest(const WebCore::ResourceRequest& request)
{
    if ([NSURLConnection canHandleRequest:request.nsURLRequest()])
        return YES;

    // FIXME: Return true if this scheme is any one WebKit2 knows how to handle.
    return request.url().protocolIs("applewebdata");
}

} // namespace WebKit
