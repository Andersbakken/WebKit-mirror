#ifndef APPLICATIONNETFLIX_H
#define APPLICATIONNETFLIX_H

#include "WebKitNetflix.h"
#include <NfApplication.h>
#include <map>

namespace WebKit {

typedef void (*TimerNetflixCallback)();

class WEBKIT_API ApplicationNetflix : public NfApplication
{
public:
    ApplicationNetflix(int& argc, char**& argv);
    ~ApplicationNetflix();

    static ApplicationNetflix* instance();

    void stopTimer(int timerid);
    int addTimer(int interval, TimerNetflixCallback callback,
                 NfApplication::TimerMode mode = NfApplication::RecurringTimer);

protected:
    void processEvent(const NfEvent* event);

private:
    struct TimerNetflixData
    {
        TimerNetflixCallback callback;
        bool single;
    };

    std::map<int, TimerNetflixData> m_timers;
};

}

#endif
