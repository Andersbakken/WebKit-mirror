#include "ApplicationNetflix.h"

namespace WebKit {

ApplicationNetflix::ApplicationNetflix(int& argc, char**& argv)
    : NfApplication(argc, argv)
{
}

ApplicationNetflix::~ApplicationNetflix()
{
}

int ApplicationNetflix::addTimer(int interval, TimerNetflixCallback callback,
                                 NfApplication::TimerMode mode)
{
    TimerNetflixData data;
    data.callback = callback;
    data.single = (mode == NfApplication::SingleTimer);
    const int id = startTimer(interval, mode);
    m_timers[id] = data;
    return id;
}

void ApplicationNetflix::processEvent(const NfEvent* event)
{
    if (event->type() != NfEvent::Timer)
        return;

    const int timerid = static_cast<const NfTimerEvent*>(event)->timerId();
    std::map<int, TimerNetflixData>::iterator it = m_timers.find(timerid);
    if (it == m_timers.end())
        return;

    (*it->second.callback)();

    // the timer could have been removed by the callback,
    // so we need to find it again
    it = m_timers.find(timerid);
    if (it == m_timers.end())
        return;
    if (it->second.single)
        m_timers.erase(it);
}

void ApplicationNetflix::stopTimer(int timerid)
{
    NfApplication::stopTimer(timerid);

    std::map<int, TimerNetflixData>::iterator it = m_timers.find(timerid);
    if (it == m_timers.end())
        return;
    m_timers.erase(it);
}

ApplicationNetflix* ApplicationNetflix::instance()
{
    return static_cast<ApplicationNetflix*>(NfApplication::instance());
}

}
