#include "timer.h"
#include "util.h"

namespace tao {

bool Timer::Comparator::operator()(const Timer::ptr& l, const Timer::ptr& r) const {
    if(!l && !r) {
        return false;
    }
    if(!l) {
        return true;
    }
    if(!r) {
        return false;
    }
    if(l->m_next < r->m_next) {
        return true;
    }
    if(r->m_next < l->m_next) {
        return false;
    }
    return l.get() < r.get();
}

Timer::Timer(uint64_t ms, std::function<void()> cb,
        bool isrecurrent, TimerManager* manager) 
    :m_isrecurrent(isrecurrent)
    ,m_ms(ms)
    ,m_cb(cb)
    ,m_manager(manager) {
    m_next = tao::GetCurrnetMS() + m_ms;
}

Timer::Timer(uint64_t next) 
    :m_next(next){

}

bool Timer::cancle() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}
    
bool Timer::refesh() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if (!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);//erase first 
    m_next = tao::GetCurrnetMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
    if (ms == m_ms && !from_now) {//if ms remains the same and not from_now
        return true;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if (from_now) {
        start = tao::GetCurrnetMS();
    } else {
        start = m_next - m_ms;//old start time
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(), lock);
    return true;
}

TimerManager::TimerManager() {

}

TimerManager::~TimerManager() {

}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin()) && !m_tickled;
    if (at_front) {
        m_tickled = true;
    }
    lock.unlock();
    if (at_front) {
        onTimerInsertedAtFront();
    }
}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()>cb
    , bool isrecurrent) {
    Timer::ptr timer(new Timer(ms, cb, isrecurrent, this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer, lock);
    return timer;
}

static void OnTimer(std::weak_ptr<void>weak_cond, std::function<void()>cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();//if weak_cond has not been free
    if (tmp) {
        cb();
    }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()>cb
                            , std::weak_ptr<void>weak_cond, bool isrecurrent) {
    return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), isrecurrent);
}

uint64_t TimerManager::getNextTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if (m_timers.empty()) {
        return ~0ull;
    }
    const Timer::ptr& next = *m_timers.begin();
    uint64_t now_ms = tao::GetCurrnetMS();
    if (now_ms >= next->m_next) //timer expired
    {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>&cbs) {
    uint64_t now_ms = tao::GetCurrnetMS();
    if (now_ms < (*m_timers.begin())->m_next) {
        return;
    }
    std::vector<Timer::ptr> expired;

    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    Timer::ptr now_tmp(new Timer(now_ms));
    auto it = m_timers.lower_bound(now_tmp);
    while (it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for (auto& timer : expired) {
        cbs.push_back(timer->m_cb);
        if (timer->m_isrecurrent) {
            timer->m_next = now_ms + timer->m_ms;
            m_timers.insert(timer);
        } else {
            timer->m_cb = nullptr;//make potential shared pointer use count --
        }
    }
}

bool TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

}