#ifndef __TAO_TIMER_H__
#define __TAO_TIMER_H__

#include <memory>
#include <functional>
#include "mutex.h"
#include <set>
#include <chrono>

namespace tao {
class TimerManager;
class ClockMonitor;

class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    using ptr = std::shared_ptr<Timer>;

    Timer(uint64_t next);

    bool cancle();
    
    bool refesh();

    /*
    ms: time interval
    from_now: true-timer start from now on, false-timer start from old time
    */
    bool reset(uint64_t ms, bool from_now);
private:
    //ms: timeout
    Timer(uint64_t ms, std::function<void()> cb,
        bool isrecurrent, TimerManager* manager);

private:
    bool m_isrecurrent = false;     //whether run as a recurrent timer
    uint64_t m_ms = 0;              //excecuting interval
    uint64_t m_next = 0;            //excecuting time stamp
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;
private:
    struct Comparator 
    {
        bool operator()(const Timer::ptr& l, const Timer::ptr& r) const;
    };
    
};

class TimerManager {
friend class Timer;
public:
    using RWMutexType = RWMutex;

    TimerManager();

    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()>cb
            , bool isrecurrent = false);

    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()>cb
                            , std::weak_ptr<void>weak_cond, bool isrecurrent = false);
    
    uint64_t getNextTimer();

    void listExpiredCb(std::vector<std::function<void()>>&cbs);

    //bool detectClockRollover(uint64_t now_ms);

    bool hasTimer();
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator>m_timers;
    bool m_tickled = false;
protected:
    virtual void onTimerInsertedAtFront() = 0;
};


class ClockMonitor {
public:
    ClockMonitor() {
        last_system_time = std::chrono::system_clock::now();
        last_steady_time = std::chrono::steady_clock::now();
    }

    std::pair<bool, std::chrono::duration<double>> checkClockChange() {
        auto current_system_time = std::chrono::system_clock::now();
        auto current_steady_time = std::chrono::steady_clock::now();

        auto system_duration = current_system_time - last_system_time;
        auto steady_duration = current_steady_time - last_steady_time;

        auto clock_difference = std::chrono::duration_cast<std::chrono::duration<double>>(system_duration - steady_duration);

        last_system_time = current_system_time;
        last_steady_time = current_steady_time;

        bool clock_changed = abs(clock_difference.count()) > tolerance.count();

        return std::make_pair(clock_changed, clock_difference);
    }

private:
    std::chrono::system_clock::time_point last_system_time;
    std::chrono::steady_clock::time_point last_steady_time;
    std::chrono::duration<double> tolerance = std::chrono::seconds(1); // Tolerance for detecting clock change
};

}

#endif