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

#include "WKBundleNodeHandle.h"
#include "WKBundleNodeHandlePrivate.h"

#include "InjectedBundleNodeHandle.h"
#include "WKAPICast.h"
#include "WKBundleAPICast.h"
#include "WebFrame.h"

using namespace WebKit;

WKTypeID WKBundleNodeHandleGetTypeID()
{
    return toAPI(InjectedBundleNodeHandle::APIType);
}

WKBundleNodeHandleRef WKBundleNodeHandleCreate(JSContextRef contextRef, JSObjectRef objectRef)
{
    RefPtr<InjectedBundleNodeHandle> nodeHandle = InjectedBundleNodeHandle::getOrCreate(contextRef, objectRef);
    return toAPI(nodeHandle.release().releaseRef());
}

WKRect WKBundleNodeHandleGetElementBounds(WKBundleNodeHandleRef nodeHandleRef)
{
    return toAPI(toImpl(nodeHandleRef)->elementBounds());
}

void WKBundleNodeHandleSetHTMLInputElementValueForUser(WKBundleNodeHandleRef htmlInputElementHandleRef, WKStringRef valueRef)
{
    toImpl(htmlInputElementHandleRef)->setHTMLInputElementValueForUser(toWTFString(valueRef));
}

bool WKBundleNodeHandleGetHTMLInputElementAutofilled(WKBundleNodeHandleRef htmlInputElementHandleRef)
{
    return toImpl(htmlInputElementHandleRef)->isHTMLInputElementAutofilled();
}

void WKBundleNodeHandleSetHTMLInputElementAutofilled(WKBundleNodeHandleRef htmlInputElementHandleRef, bool filled)
{
    toImpl(htmlInputElementHandleRef)->setHTMLInputElementAutofilled(filled);
}

WKBundleNodeHandleRef WKBundleNodeHandleCopyHTMLTableCellElementCellAbove(WKBundleNodeHandleRef htmlTableCellElementHandleRef)
{
    RefPtr<InjectedBundleNodeHandle> nodeHandle = toImpl(htmlTableCellElementHandleRef)->htmlTableCellElementCellAbove();
    return toAPI(nodeHandle.release().releaseRef());
}

WKBundleFrameRef WKBundleNodeHandleCopyDocumentFrame(WKBundleNodeHandleRef documentHandleRef)
{
    RefPtr<WebFrame> frame = toImpl(documentHandleRef)->documentFrame();
    return toAPI(frame.release().releaseRef());
}

WKBundleFrameRef WKBundleNodeHandleCopyHTMLFrameElementContentFrame(WKBundleNodeHandleRef htmlFrameElementHandleRef)
{
    RefPtr<WebFrame> frame = toImpl(htmlFrameElementHandleRef)->htmlFrameElementContentFrame();
    return toAPI(frame.release().releaseRef());
}

WKBundleFrameRef WKBundleNodeHandleCopyHTMLIFrameElementContentFrame(WKBundleNodeHandleRef htmlIFrameElementHandleRef)
{
    RefPtr<WebFrame> frame = toImpl(htmlIFrameElementHandleRef)->htmlIFrameElementContentFrame();
    return toAPI(frame.release().releaseRef());
}
