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

#ifndef PageClient_h
#define PageClient_h

#include "WebPageProxy.h"
#include "WebPopupMenuProxy.h"
#include <wtf/Forward.h>

namespace WebCore {
    class Cursor;
    struct ViewportArguments;
}

namespace WebKit {

class FindIndicator;
class NativeWebKeyboardEvent;
class NativeWebKeyboardEvent;
class WebContextMenuProxy;
class WebEditCommandProxy;
class WebPopupMenuProxy;

class PageClient {
public:
    virtual ~PageClient() { }

    virtual void processDidCrash() = 0;
    virtual void didRelaunchProcess() = 0;

    virtual void takeFocus(bool direction) = 0;
    virtual void toolTipChanged(const String&, const String&) = 0;

#if ENABLE(TILED_BACKING_STORE)
    virtual void pageDidRequestScroll(const WebCore::IntSize&) = 0;
#endif
#if PLATFORM(QT)
    virtual void didChangeContentsSize(const WebCore::IntSize&) = 0;
    virtual void didFindZoomableArea(const WebCore::IntRect&) = 0;
#endif

    virtual void setCursor(const WebCore::Cursor&) = 0;
    virtual void setViewportArguments(const WebCore::ViewportArguments&) = 0;

    virtual void registerEditCommand(PassRefPtr<WebEditCommandProxy>, WebPageProxy::UndoOrRedo) = 0;
    virtual void clearAllEditCommands() = 0;
    virtual void setEditCommandState(const String& commandName, bool isEnabled, int state) = 0;
#if PLATFORM(MAC)
    virtual void interceptKeyEvent(const NativeWebKeyboardEvent&, Vector<WebCore::KeypressCommand>&, uint32_t, uint32_t, Vector<WebCore::CompositionUnderline>&) = 0;
#endif
#if PLATFORM(WIN)
    virtual void compositionSelectionChanged(bool) = 0;
#endif
    virtual WebCore::FloatRect convertToDeviceSpace(const WebCore::FloatRect&) = 0;
    virtual WebCore::FloatRect convertToUserSpace(const WebCore::FloatRect&) = 0;

    virtual void didNotHandleKeyEvent(const NativeWebKeyboardEvent&) = 0;

    virtual PassRefPtr<WebPopupMenuProxy> createPopupMenuProxy(WebPageProxy*) = 0;
    virtual PassRefPtr<WebContextMenuProxy> createContextMenuProxy(WebPageProxy*) = 0;

    virtual void setFindIndicator(PassRefPtr<FindIndicator>, bool fadeOut) = 0;

#if USE(ACCELERATED_COMPOSITING)
    virtual void pageDidEnterAcceleratedCompositing() = 0;
    virtual void pageDidLeaveAcceleratedCompositing() = 0;
#endif

#if PLATFORM(WIN)
    virtual HWND nativeWindow() = 0;
#endif

#if PLATFORM(MAC)
    virtual void setComplexTextInputEnabled(uint64_t pluginComplexTextInputIdentifier, bool complexTextInputEnabled) = 0;
#endif

    // Custom representations.
    virtual void didCommitLoadForMainFrame(bool useCustomRepresentation) = 0;
    virtual void didFinishLoadingDataForCustomRepresentation(const CoreIPC::DataReference&) = 0;
};

} // namespace WebKit

#endif // PageClient_h
