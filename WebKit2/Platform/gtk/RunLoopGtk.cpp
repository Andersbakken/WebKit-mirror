/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Portions Copyright (c) 2010 Motorola Mobility, Inc.  All rights reserved.
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

#include "RunLoop.h"

#include "WKBase.h"
#include <glib.h>

RunLoop::RunLoop()
{
    m_runLoopContext = g_main_context_default();
    ASSERT(m_runLoopContext);
    m_runLoopMain = g_main_loop_new(m_runLoopContext, FALSE);
    ASSERT(m_runLoopMain);
}

RunLoop::~RunLoop()
{
    if (m_runLoopMain) {
        g_main_loop_quit(m_runLoopMain);
        g_main_loop_unref(m_runLoopMain);
    }

    if (m_runLoopContext)
        g_main_context_unref(m_runLoopContext);
}

void RunLoop::run()
{
    g_main_loop_run(RunLoop::main()->mainLoop());
}

GMainLoop* RunLoop::mainLoop()
{
    return m_runLoopMain;
}

void RunLoop::stop()
{
    g_main_loop_quit(m_runLoopMain);
}

gboolean RunLoop::queueWork(RunLoop* runLoop)
{
    runLoop->performWork();
    return FALSE;
}

void RunLoop::wakeUp()
{
    GSource* source = g_timeout_source_new(0);
    g_source_set_callback(source, reinterpret_cast<GSourceFunc>(&RunLoop::queueWork), this, 0);
    g_source_attach(source, m_runLoopContext);

    g_main_context_wakeup(m_runLoopContext);
}

RunLoop::TimerBase::TimerBase(RunLoop* runLoop)
    : m_runLoop(runLoop)
    , m_timerSource(0)
{
}

RunLoop::TimerBase::~TimerBase()
{
    stop();
}

void RunLoop::TimerBase::resetTimerSource()
{
    m_timerSource = 0;
}

gboolean RunLoop::TimerBase::oneShotTimerFired(RunLoop::TimerBase* timer)
{
    timer->fired();
    timer->resetTimerSource();
    return FALSE;
}

gboolean RunLoop::TimerBase::repeatingTimerFired(RunLoop::TimerBase* timer)
{
    timer->fired();
    return TRUE;
}

void RunLoop::TimerBase::start(double nextFireInterval, double repeatInterval)
{
    if (m_timerSource)
        stop();

    if (repeatInterval) {
        m_timerSource = g_timeout_source_new(static_cast<guint>(repeatInterval));
        g_source_set_callback(m_timerSource, reinterpret_cast<GSourceFunc>(&RunLoop::TimerBase::repeatingTimerFired), this, 0);
    } else {
        m_timerSource = g_timeout_source_new(static_cast<guint>(nextFireInterval));
        g_source_set_callback(m_timerSource, reinterpret_cast<GSourceFunc>(&RunLoop::TimerBase::oneShotTimerFired), this, 0);
    }
    g_source_attach(m_timerSource, m_runLoop->m_runLoopContext);
}

void RunLoop::TimerBase::stop()
{
    if (!m_timerSource)
        return;

    g_source_destroy(m_timerSource);
    m_timerSource = 0;
}

bool RunLoop::TimerBase::isActive() const
{
    return m_timerSource;
}
