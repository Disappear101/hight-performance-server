#ifndef __TAO_IOMANAGER_H__
#define __TAO_IOMANAGER_H__

#include "scheduler.h"

namespace tao {

class IOManager : public Scheduler {
public:
    using ptr = std::shared_ptr<IOManager>;
    using RWMutexType = RWMutex;

    enum Event {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4
    };
private:
    struct FdContext
    {  
        using MutexType = Mutex;
        struct EventContext
        {
            Scheduler* scheduler;       //event handler scheduler
            Fiber::ptr fiber;           //event fiber
            std::function<void()>cb;    //event call back
        };

        int fd;
        EventContext read;      //read event
        EventContext write;     //write event
        Event m_events = NONE;  //fd of event
        MutexType mutex;

    };
public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = std::string());
    ~IOManager();

    //1 success, 0, retry, -1 error
    int addEvent(int fd, Event e, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event e);
    bool cancelEvent(int fd, Event e);

    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    
private:
    int m_epfd = 0;
    
};
}

#endif