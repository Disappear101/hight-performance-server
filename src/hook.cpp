#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "fdmanager.h"
#include "timer.h"
#include <dlfcn.h>

tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

namespace tao {
static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }
//define function pointers
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);//Expanding this macro results in: socket_f = (socket_fun)dlsym(RTLD_NEXT, "socket");
    HOOK_FUN(XX);
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hook_init();
    }
};

static _HookIniter s_hook_initer;


bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

}

struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
            uint32_t event, int timeout_so, Args&&... args) {
    if (!tao::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(fd);
    if (!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonBlock()) {//non socket or user set as non block
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();

retry://socket io operation
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while (n == -1 && errno == EINTR) {//EINTR: interrupt. when it is interruptedm, retry
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) {//EAGAIN: io block
        tao::IOManager* iom = tao::IOManager::GetThis();
        tao::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {//has time out
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;//110
                iom->cancelEvent(fd, (tao::IOManager::Event)(event));//cancel 
            }, winfo);
        }

        int rt = iom->addEvent(fd, (tao::IOManager::Event)(event));//add socket io event
        if (rt) {
            TAO_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            tao::Fiber::YieldToHold();//swap to scheduler fiber, set status as HOLD
            if (timer) {//if socket io is activated, wake up here, cancle the timer 
                timer->cancel();
            }
            if (tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }

    return n;
}

extern "C" {
//declare function pointers
#define XX(name) name ## _fun name ## _f = nullptr;//Expanding this macro results in: socket_fun socket_f = nullptr;
    HOOK_FUN(XX);
#undef XX

//fulfill asyn sleep hook
unsigned int sleep(unsigned int seconds) {
    if (!tao::t_hook_enable) {
        return sleep_f(seconds);
    }

    tao::Fiber::ptr fiber = tao::Fiber::GetThis();
    tao::IOManager* iom = tao::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(tao::Scheduler::*)
            (tao::Fiber::ptr, int thread))&tao::IOManager::schedule
            ,iom, fiber, -1));//trick: cast template function to a specified function pointer
    tao::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if (!tao::t_hook_enable) {
        return usleep(usec);
    }
    tao::Fiber::ptr fiber = tao::Fiber::GetThis();
    tao::IOManager* iom = tao::IOManager::GetThis();
    iom->addTimer(usec * 1000, std::bind((void(tao::Scheduler::*)
            (tao::Fiber::ptr, int thread))&tao::IOManager::schedule
            ,iom, fiber, -1));
    tao::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!tao::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    tao::Fiber::ptr fiber = tao::Fiber::GetThis();
    tao::IOManager* iom = tao::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(tao::Scheduler::*)
            (tao::Fiber::ptr, int thread))&tao::IOManager::schedule
            ,iom, fiber, -1));
    tao::Fiber::YieldToHold();
    return 0;
}

}
