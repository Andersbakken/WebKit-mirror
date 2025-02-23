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

#if ENABLE(TOUCH_EVENTS)

#include "WebEvent.h"

#include "ArgumentCoders.h"
#include "Arguments.h"

namespace WebKit {

WebTouchEvent::WebTouchEvent(WebEvent::Type type, Vector<WebPlatformTouchPoint> touchPoints, bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, Modifiers modifiers, double timestamp)
    : WebEvent(type, modifiers, timestamp)
    , m_touchPoints(touchPoints)
    , m_ctrlKey(ctrlKey)
    , m_altKey(altKey)
    , m_shiftKey(shiftKey)
    , m_metaKey(metaKey)
{
    ASSERT(isTouchEventType(type));
}

void WebTouchEvent::encode(CoreIPC::ArgumentEncoder* encoder) const
{
    WebEvent::encode(encoder);

    encoder->encode(CoreIPC::In(m_touchPoints));
}

bool WebTouchEvent::decode(CoreIPC::ArgumentDecoder* decoder, WebTouchEvent& t)
{
    if (!WebEvent::decode(decoder, t))
        return false;

    return decoder->decode(CoreIPC::Out(t.m_touchPoints));
}

bool WebTouchEvent::isTouchEventType(Type type)
{
    return type == TouchStart || type == TouchMove || type == TouchEnd || type == TouchCancel;
}
    
} // namespace WebKit

#endif // ENABLE(TOUCH_EVENTS)
