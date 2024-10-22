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

#include "Arguments.h"
#include "WebCoreArgumentCoders.h"

using namespace WebCore;

namespace WebKit {

WebPlatformTouchPoint::WebPlatformTouchPoint(unsigned id, TouchPointState state, const IntPoint& screenPosition, const IntPoint& position)
    : m_id(id)
    , m_state(state)
    , m_screenPosition(screenPosition)
    , m_position(position)
{
}

void WebPlatformTouchPoint::encode(CoreIPC::ArgumentEncoder* encoder) const
{
    encoder->encode(CoreIPC::In(m_id, m_state, m_screenPosition, m_position));
}

bool WebPlatformTouchPoint::decode(CoreIPC::ArgumentDecoder* decoder, WebPlatformTouchPoint& t)
{
    return decoder->decode(CoreIPC::Out(t.m_id, t.m_state, t.m_screenPosition, t.m_position));
}

} // namespace WebKit

#endif // ENABLE(TOUCH_EVENTS)
