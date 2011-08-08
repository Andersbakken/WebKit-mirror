
#include "config.h"

#include "EventLoopNetflix.h"
#include "ResourceHandleManager.h"

#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fcntl.h>

namespace WebCore {
void checkSharedTimer();
}

namespace WebKit {

static EventLoopNetflix *sEventLoop = 0;

EventLoopNetflix::EventLoopNetflix()
{
    assert(!sEventLoop);
    sEventLoop = this;
    mSharedTimerInterval = -1;
    mNextSharedTimer.tv_sec = 0;
    if (pipe(mWakeup) == -1) {
        mWakeup[0] = mWakeup[1] = -1;
    } else {
        initWakeupPipe(mWakeup[0]);
        initWakeupPipe(mWakeup[1]);
    }
}

EventLoopNetflix::~EventLoopNetflix()
{
    close(mWakeup[0]);
    close(mWakeup[1]);
    assert(sEventLoop == this);
    sEventLoop = 0;
}

EventLoopNetflix *EventLoopNetflix::sharedInstance()
{
    assert(sEventLoop);
    return sEventLoop;
}

void EventLoopNetflix::execute()
{
    while(true) {
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = 0;
        if(WebCore::ResourceHandleManager *resources = WebCore::ResourceHandleManager::sharedInstance())
            resources->getFileDescriptors(&fdread, &fdwrite, &fdexcep, &maxfd);
        FD_SET(mWakeup[0], &fdread);
        if(mWakeup[0] > maxfd)
            maxfd = mWakeup[0];

        int s = 0;
        if(mNextSharedTimer.tv_sec) {
            struct timeval tv_now;
            gettimeofday(&tv_now, 0);
            struct timeval tv_wait;
            timersub(&mNextSharedTimer, &tv_now, &tv_wait);
            if(tv_wait.tv_sec < 0) { //time passed already
                tv_wait.tv_sec = 0;
                tv_wait.tv_usec = 0;
            }
            s = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &tv_wait);
        } else {
            s = select(maxfd+1, &fdread, &fdwrite, &fdexcep, 0);
        }
#if 1
        if (s == -1) {
            perror("EventLoopNetflix::execute");
            break;
        }
#endif
        if(s >= 1 && FD_ISSET(mWakeup[0], &fdread)) {
            --s;
            char b;
            while(true) {
                const int r = read(mWakeup[0], &b, 1);
                if(r <= 0)
                    break;
                if(r == 1 && b == 'q')
                    return;
            }
        }
        if(s >= 1)
            notify(new EventNetflix(EventNetflix::ResourceHandleJobs));
        struct timeval tv_now;
        gettimeofday(&tv_now, 0);
        if(mNextSharedTimer.tv_sec && timercmp(&tv_now, &mNextSharedTimer, >=)) {
            notify(new EventNetflix(EventNetflix::SharedTimer));
#if 0
            updateNextSharedTimer();
#else
            mNextSharedTimer.tv_sec = 0;
#endif
        }
    }
}

void EventLoopNetflix::wakeup()
{
    const char data = 'w';
    while(::write(mWakeup[1], &data, 1) != 1);
}

void EventLoopNetflix::quit()
{
    const char data = 'q';
    while(::write(mWakeup[1], &data, 1) != 1);
}

void EventLoopNetflix::notify(EventNetflix *event)
{
    switch(event->type()) {
    case EventNetflix::ResourceHandleJobs: {
        if(WebCore::ResourceHandleManager *resources = WebCore::ResourceHandleManager::sharedInstance())
            resources->processJobs();
        break; }
    case EventNetflix::SharedTimer: {
        WebCore::checkSharedTimer();
        updateNextSharedTimer();
        break; }
    default:
        break;
    }
    delete event;
}

void EventLoopNetflix::setSharedTimerInterval(double interval)
{
    mSharedTimerInterval = interval;
    updateNextSharedTimer();
}

void EventLoopNetflix::updateNextSharedTimer()
{
    if(mSharedTimerInterval == -1) {
        mNextSharedTimer.tv_sec = 0;
    } else {
        struct timeval tv_now;
        gettimeofday(&tv_now, 0);
        struct timeval tv_interval;
        tv_interval.tv_sec = floor(mSharedTimerInterval);
        tv_interval.tv_usec = 1e6 * (mSharedTimerInterval - tv_interval.tv_sec);
        timeradd(&tv_now, &tv_interval, &mNextSharedTimer);
        WebKit::EventLoopNetflix::sharedInstance()->wakeup();
    }
}

void EventLoopNetflix::initWakeupPipe(int fd)
{
    const int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        perror("EventLoopNetflix: Unable to get flags on wakup pipe");
    const int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
        perror("EventLoopNetflix: Unable to set flags on wakup pipe");
}

}
