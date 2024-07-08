#ifndef __TAO_IOMANAGER_H__
#define __TAO_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"
#include <sys/epoll.h>

namespace tao {

class IOManager : public Scheduler , public TimerManager {
public:
    using ptr = std::shared_ptr<IOManager>;
    using RWMutexType = RWMutex;

    enum Event {
        NONE = 0x0,
        READ = 0x1,     //EPOLLOUT
        WRITE = 0x4     //EPOLLIN
    };
private:
    struct FdContext
    {  
        using MutexType = Mutex;
        struct EventContext
        {
            Scheduler* scheduler = nullptr;       //event handler scheduler
            Fiber::ptr fiber;           //event fiber
            std::function<void()>cb;    //event call back
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);

        int fd = 0;
        EventContext read;      //read event
        EventContext write;     //write event
        Event m_events = NONE;  //fd of event
        MutexType mutex;

    };
public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = std::string());
    ~IOManager();

    //0 success, -1 error
    int addEvent(int fd, Event e, std::function<void()> cb = nullptr);
    //delete specified events
    bool delEvent(int fd, Event e);
    //cancel specified events
    bool cancelEvent(int fd, Event e);
    //cancel all events
    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void contextResize(size_t sz);
    
    void onTimerInsertedAtFront() override;

    bool stopping(uint64_t& timeout);
private:
    int m_epfd = 0;
    int m_tickleFds[2];//pipe

    std::atomic<size_t> m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;

};



}

#endif