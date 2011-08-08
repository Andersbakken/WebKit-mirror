#ifndef __EVENTLOOPNETFLIX_H__
#define __EVENTLOOPNETFLIX_H__

#include "WebKitNetflix.h"

#include <sys/time.h>

namespace WebKit {

class EventNetflix
{
public:
    enum Type {
        LoadFail,
        WindowClose,
        WindowClear,
        FrameCreate,
        SharedTimer,
        ResourceHandleJobs
    };
    EventNetflix(Type t) : m_type(t) { }
    virtual ~EventNetflix() { }

    Type type() const { return m_type; };
private:
    Type m_type;
};

class WEBKIT_API EventLoopNetflix
{
public:
    EventLoopNetflix();
    virtual ~EventLoopNetflix();

    static EventLoopNetflix *sharedInstance();

    void execute();

    void setSharedTimerInterval(double d);

    void wakeup();
    void quit();

protected:
    virtual void notify(WebKit::EventNetflix *event);

private:
    void initWakeupPipe(int fd);
    void updateNextSharedTimer();
    double mSharedTimerInterval;
    timeval mNextSharedTimer;
    int mWakeup[2];
};

}

#endif /* __EVENTLOOPNETFLIX_H__ */
