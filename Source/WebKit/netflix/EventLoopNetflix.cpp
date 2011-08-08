
#include "EventLoopNetflix.h"

namespace WebCore {
void checkSharedTimer(double *nextInterval);
bool getSharedTimer(double &interval);
}

namespace WebKit {

EventLoopNetflix::EventLoopNetflix()
{
    mSharedTimerInterval = 0;
    mNextSharedTimer.tv_sec = 0;
    if (pipe(mWakeup) == -1)
        mWakeup[0] = mWakeup[1] = -1;
}

EventLoopNetflix::~EventLoopNetflix()
{
    close(mWakeup[0]);
    close(mWakeup[1]);
}

EventLoopNetflix *EventLoopNetflix::sharedInstance()
{
    static EventLoopNetflix* sharedInstance = 0;
    if (!sharedInstance)
        sharedInstance = new EventLoopNetflix();
    return sharedInstance;
}

void EventLoopNetflix::execute()
{
    while(true) {
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = 0;
        if(ResourceHandleManager *resources = ResourceHandleManager::sharedInstance())
            resources->getFileDescriptors(&fdread, &fdwrite, &fdexcep, &maxfd);
        FD_SET(mWakeup[0], fdread);
        if(fd > maxfd)
            maxfd = fd;

        int s = 0;
        if(mNextSharedTimer.tv_sec) {
            struct timeval tv_now;
            gettimeofday(&tv_now, 0);
            struct timeval tv_wait;
            timersub(&tv_now, &mNextSharedTimer, &tv_wait);
            s = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &tv_wait);
        } else {
            s = select(maxfd+1, &fdread, &fdwrite, &fdexcep, 0);
        }
        if (s == -1)
            break;
        if(s >= 1 && FD_ISSET(mWakeup[0], &read_fds)) {
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
#if 1
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
    ::write(mWakeup[1], &data, 1);
}

void EventLoopNetflix::quit()
{
    const char data = 'q';
    ::write(mWakeup[1], &data, 1);
}

void EventLoopNetflix::process(EventNetflix *event)
{
    switch(event->type()) {
    case ResourceHandleJobs: {
        if(ResourceHandleManager *resources = ResourceHandleManager::sharedInstance())
            resources->processJobs();
        break; }
    case EventNetflix::SharedTimer: {
        WebCore::checkSharedTimer();
        updateNextSharedTimer();
        break; }
    default:
        break;
    }
}

void EventLoopNetflix::notify(EventNetflix *event)
{
     process(event);
     delete event;
}

}

void EventLoopNetflix::setSharedTimerInterval(double interval)
{
    mSharedTimerInterval = interval;
    updateNextSharedTimer();
}

void EventLoopNetflix::updateNextSharedTimer()
{
    struct timeval tv_now;
    gettimeofday(&tv_now, 0);
    struct timeval tv_interval;
    tv_interval.tv_sec = floor(interval);
    tv_interval.tv_usec = 1e6 * (interval - tv_interval.tv_sec);
    timeradd(&tv_now, &tv_interval, &mNextSharedTimer);
}
