/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2009 Adam Barth. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NavigationScheduler_h
#define NavigationScheduler_h

#include "Event.h"
#include "Timer.h"
#include <wtf/Forward.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/PassRefPtr.h>

namespace WebCore {

class FormState;
class FormSubmission;
class Frame;
class ScheduledNavigation;
class SecurityOrigin;

struct FrameLoadRequest;

class NavigationScheduler : public Noncopyable {
public:
    NavigationScheduler(Frame*);
    ~NavigationScheduler();

    bool redirectScheduledDuringLoad();
    bool locationChangePending();

    void scheduleRedirect(double delay, const String& url);
    void scheduleLocationChange(PassRefPtr<SecurityOrigin>, const String& url, const String& referrer, bool lockHistory = true, bool lockBackForwardList = true);
    void scheduleFormSubmission(PassRefPtr<FormSubmission>);
    void scheduleRefresh();
    void scheduleHistoryNavigation(int steps);

    void startTimer();

    void cancel(bool newLoadInProgress = false);
    void clear();

private:
    void timerFired(Timer<NavigationScheduler>*);
    void schedule(PassOwnPtr<ScheduledNavigation>);

    static bool mustLockBackForwardList(Frame* targetFrame);

    Frame* m_frame;
    Timer<NavigationScheduler> m_timer;
    OwnPtr<ScheduledNavigation> m_redirect;
};

} // namespace WebCore

#endif // NavigationScheduler_h
