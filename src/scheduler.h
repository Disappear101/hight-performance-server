#ifndef __TAO_SCHEDULER_H__
#define __TAO_SCHEDULER_H__

#include <memory>
#include <functional>
#include "mutex.h"
#include "thread.h"
#include "fiber.h"

namespace tao {

class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = Mutex;

    /*
    nthd: number of thread
    use_caller: 
    name: name of Scheduler
    */
    Scheduler(size_t nthd = 1, bool use_caller = true, const std::string& name = std::string());
    virtual ~Scheduler();

    const std::string& getName() const { return m_name;}

    /*
    get current Scheduler
    */
    static Scheduler* GetThis();
    /*
    get scheduler fiber used for Scheduling in a thread
    */
    static Fiber* GetSchedulerFiber();

    void start();
    void stop();

    //specifying fiber or cb run in a thread
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }
        if (need_tickle) {
            tickle();
        }
    }

    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;//&*begin get address of cur val
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }
protected:
    //wake up all
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();

    void setThis();

private:
    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    //fiber/cb and thread pair, meaning fiber/cb will run on the specified thread
    struct FiberAndThread
    {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f), thread(thr) {
        }
        FiberAndThread(Fiber::ptr* f, int thr)
            :thread(thr) {
            fiber.swap(*f);
        }
        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }
        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }
        FiberAndThread() 
            :thread(-1) {
        }

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
    
private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;
    Fiber::ptr m_rootFiber;//scheduler fiber of thread
    std::string m_name;

protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = 0;
    bool m_stopping = true;//
    bool m_autoStop = false;//subjecttively stop
    int m_rootThread = 0;//main thread 
};

}

#endif