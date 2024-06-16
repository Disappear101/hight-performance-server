#include "iomanager.h"
#include "macro.h"
#include "log.h"
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <string>

namespace tao {
static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event) {
    switch (event)
    {
    case IOManager::READ:
        return read; 
    case IOManager::WRITE:
        return write;
    default:
        TAO_ASSERT2(false, "getContext");
    }
}
void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}
void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    TAO_ASSERT(m_events & event);
    m_events = (Event) (m_events & !event);
    EventContext& ctx = getContext(event);
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    :Scheduler(threads, use_caller, name){
    m_epfd = epoll_create1(0);
    TAO_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);//intitiate bidirectional pipe(0:read, 1:write)
    TAO_ASSERT(rt == 0);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN |  EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    TAO_ASSERT(rt == 0);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    TAO_ASSERT(rt == 0);

    contextResize(32);

    start();
}
IOManager::~IOManager() {
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

//0 success, -1 error
int IOManager::addEvent(int fd, Event e, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
    } else {
        lock.unlock();
        RWMutex::WriteLock lock2(m_mutex);
        contextResize(m_fdContexts.size() * 1.5);
        fd_ctx = m_fdContexts[fd];
    } 

    FdContext::MutexType::Lock lock3(fd_ctx->mutex);
    if (fd_ctx->m_events & e) {//if fd already exists and have the same events. It should not happen
        TAO_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd
            << " event = " << e
            << " fd_ctx.event" << fd_ctx->m_events;
        TAO_ASSERT(!(fd_ctx->m_events & e));
    }
    int op = fd_ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->m_events | e;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        TAO_LOG_ERROR(g_logger) << "epoll_ctl ()" << m_epfd << ", "
                << op << ", " << fd << ", " << epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return -1;
    }
    ++m_pendingEventCount;
    fd_ctx->m_events = (Event) (fd_ctx->m_events | e);//update events
    FdContext::EventContext& event_ctx = fd_ctx->getContext(e);
    TAO_ASSERT(!event_ctx.scheduler 
                && !event_ctx.fiber
                && !event_ctx.cb);
    
    event_ctx.scheduler = Scheduler::GetThis();
    if (cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
        TAO_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event e) {
    RWMutex::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];//fetch specified fd context
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if (!(fd_ctx->m_events & e)) {//if fd doesn't exist
        return false;
    }

    Event new_events = (Event) (fd_ctx->m_events & ~e);//del events from e and remain orginal events
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        TAO_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
                << op << ", " << fd << ", " << epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    --m_pendingEventCount;
    fd_ctx->m_events = new_events;
    FdContext::EventContext& even_ctx = fd_ctx->getContext(e);
    fd_ctx->resetContext(even_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd, Event e) {
    RWMutex::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];//fetch specified fd context
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if (!(fd_ctx->m_events & e)) {//if fd doesn't exist
        return false;
    }

    Event new_events = (Event) (fd_ctx->m_events & ~e);//del events from e and remain orginal events
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        TAO_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
                << op << ", " << fd << ", " << epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    //FdContext::EventContext& even_ctx = fd_ctx->getContext(e);
    fd_ctx->triggerEvent(e);
    --m_pendingEventCount;
    // fd_ctx->m_events = new_events;
    // fd_ctx->resetContext(even_ctx);
    return true;
}

bool IOManager::cancleAll(int fd) {
    RWMutex::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];//fetch specified fd context
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if (!(fd_ctx->m_events)) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        TAO_LOG_ERROR(g_logger) << "epoll_ctl" << m_epfd << ", "
                << op << ", " << fd << ", " << epevent.events << "):"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    if (fd_ctx->m_events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if (fd_ctx->m_events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }
    TAO_ASSERT(fd_ctx->m_events == 0);
    return true;
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
    if (!hasIdleThreads()) {
        return;
    }
    int rt = write(m_tickleFds[1], "T", 1);// Write a single byte to wake up the epoll_wait
    TAO_ASSERT(rt == 1);
}

bool IOManager::stopping() {

    return Scheduler::stopping()
        && m_pendingEventCount == 0;
}

void IOManager::idle() {
    TAO_LOG_DEBUG(g_logger) << "idle";
    const uint64_t MAX_EVNETS = 256;
    epoll_event* events = new epoll_event[MAX_EVNETS]();
    std::shared_ptr<epoll_event> epevents(events,
        [](epoll_event* p) {
            delete[] p;
            //std::cout << "Custom array deleter called\n";
        });

    while (true) {
        if (stopping()) {
            TAO_LOG_INFO(g_logger) << "name = " << getName()
                            << " idle stopping exit";
            break;  
        }

        int rt = 0;
        do {
            static const int MAX_TIMEOUT = 5000;
            rt = epoll_wait(m_epfd, events, MAX_EVNETS, MAX_TIMEOUT);

            if (rt < 0 && errno == EINTR) {//errno == EINTR means that os interrupt before data arrival. try agin

            } else {
                break;
            }   
        } while (true);

        //handle active events
        for (int i = 0; i < rt; ++i) {
            epoll_event& event = events[i];
            if (event.data.fd == m_tickleFds[0]) {
                uint8_t dummy[256];
                while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);//Clear the pipe. Level trigger, use while to read all
                continue;
            }
            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {//when error and hup
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->m_events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->m_events & real_events) == NONE) {
                continue;
            }

            int left_events = (fd_ctx->m_events & ~real_events);//del real events
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                TAO_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();//switch to scheduler fiber. jump out idle to task fiber
    }
}

void IOManager::contextResize(size_t sz) {
    m_fdContexts.resize(sz);

    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

}