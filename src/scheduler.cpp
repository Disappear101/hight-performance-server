#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace tao {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");


static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t nthd, bool use_caller, const std::string& name) 
    :m_name(name) {
    TAO_ASSERT(nthd > 0);

    if (use_caller) {
        //create a main fiber for current thread
        tao::Fiber::GetThis();
        --nthd;//current thread is condsidered as an element in the threadpool

        TAO_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        //reset scheluler fiber of current thread
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        tao::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = tao::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = nthd;
}

Scheduler::~Scheduler() {
    TAO_ASSERT(m_stopping);
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetSchedulerFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if (!m_stopping) {
        return;
    }
    m_stopping = false;

    TAO_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i] = std::make_shared<tao::Thread>(std::bind(&Scheduler::run, this)
                                                            , m_name + "_" + std::to_string(i));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();//unlock here, prevent deadlock from happening, because context switching will happen in call function

    // for(size_t i = 0; i < m_threadCount; ++i) {
    //     tickle();
    // }

    // if (m_rootFiber) {
    //     if (!stopping()) m_rootFiber->call();
    // }

    // std::vector<Thread::ptr> thrs;
    // {
    //     MutexType::Lock lock(m_mutex);
    //     thrs.swap(m_threads);
    // }

    // for(auto& i : thrs) {
    //     i->join();
    // }

}
void Scheduler::stop() {
    m_autoStop = true;
    if (m_rootFiber && m_threadCount == 0 
                && (m_rootFiber->getState() == Fiber::TERM 
                    || m_rootFiber->getState() == Fiber::INIT)) {
        TAO_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;
        if (stopping()) {
            return;
        }
    }
    
    if (m_rootThread != -1) {
        TAO_ASSERT(GetThis() == this);
    } else {
        TAO_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    //wake up all threads to finish execution
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }
    //if use_caller, wake up caller thread to finish execution
    if(m_rootFiber) {
        tickle();
    }

    if (m_rootFiber) {
        if (!stopping()) {
            m_rootFiber->call();//
        }
    }
    
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
}

void Scheduler::tickle() {
    TAO_LOG_INFO(g_logger) << "tickle";
}

void Scheduler::run() {
    TAO_LOG_DEBUG(g_logger) << m_name << " run";
    set_hook_enable(true);
    setThis();
    //when use_caller = false, initiate t_scheduler_fiber
    if (tao::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    //create an idle fiber and a cb fiber for cb task
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while (true)
    {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                //when thread id is non-current thread id
                if (it->thread != -1 && it->thread != tao::GetThreadId()) {
                    ++it;
                    tickle_me = true;//invoke other thread to handle
                    continue;
                }
                TAO_ASSERT(it->fiber || it->cb);
                //when target has been excecuted
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                //fetch one task that specified executing thread is current thread.
                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }

        if(tickle_me) {
            tickle();
        }

        //when task is defined in fiber mode and not under TERN AND EXCEPT status
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();//switch to sub fiber 
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber); //reschedule
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->setState(Fiber::HOLD); 
            }
            ft.reset();
        } else if(ft.cb) { //when task is defined in cb mode
            //initiate fiber according to cb
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                cb_fiber->setState(Fiber::HOLD);
                cb_fiber.reset();
            }
        } else { //when no pendding task to be handled
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                TAO_LOG_INFO(g_logger) << "idle fiber term";
                //idle_fiber.reset();
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
    

}
bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    TAO_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        tao::Fiber::YieldToHold();
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}


}