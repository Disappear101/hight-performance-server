#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "fdmanager.h"
#include "config.h"
#include "timer.h"
#include "macro.h"
#include <dlfcn.h>
#include <cstdarg>

tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

namespace tao {

static tao::ConfigVar<int>::ptr g_tcp_connect_timeout = 
    tao::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

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

static uint64_t s_connect_timeout = -1;

struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();
        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
            TAO_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                            << old_value << " to " << new_value;
            s_connect_timeout = new_value;
        });
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

    int flags = fcntl_f(fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK)) {
        fcntl_f(fd, F_SETFL, flags | O_NONBLOCK);//set as non-block
        TAO_LOG_DEBUG(g_logger) << "Socket is reset as non-blocking";//trigger, when reconnection is happening
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();

retry://socket io operation
    ssize_t n = fun(fd, std::forward<Args>(args)...);//bug here to solve: it becomes blocked io somehow. 
    while (n == -1 && errno == EINTR) {//EINTR: interrupt. when it is interruptedm, retry
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) {//EAGAIN: data is not coming yet, try again
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
            tao::Fiber::YieldToHold();//swap to scheduler fiber to schedule io event, set status as HOLD
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


int socket(int domain, int type, int protocol) {
    if (!tao::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        return fd;
    } 
    tao::FdMgr::GetInstance()->get(fd, true);//register fd and set as non-block
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms) {
    if (!tao::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(fd);
    if (!ctx || ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen); 
    }
    if (ctx->getUserNonBlock()) {
        return connect_f(fd, addr, addrlen); 
    }

    int n = connect_f(fd, addr, addrlen);
    if (n == 0) {
        return 0;
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    tao::IOManager* iom = tao::IOManager::GetThis();
    tao::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();
    std::weak_ptr<timer_info> winfo(tinfo);

    if (timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, fd, iom](){
            auto t = winfo.lock();
            if (!t || t->cancelled) {
                return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(fd, tao::IOManager::WRITE);
            TAO_LOG_INFO(g_logger) << "connect timeout, connect event canceled";
        }, winfo);
    }

    int rt = iom->addEvent(fd, tao::IOManager::WRITE);
    if (rt == 0) {
        tao::Fiber::YieldToHold();
        if (timer) {
            timer->cancel();
        }
        if (tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        TAO_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, tao::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", tao::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if (fd > 0) {
        tao::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!tao::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(sockfd);
            if (ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", tao::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", tao::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", tao::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", tao::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", tao::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", tao::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", tao::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", tao::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", tao::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", tao::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if (!tao::t_hook_enable) {
        return close_f(fd);
    }
    tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(fd);
    if (ctx) {
        auto iom = tao::IOManager::GetThis();
        if (iom) {
            iom->cancelAll(fd);
        }
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch (cmd)    
    {
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(fd);
            if (!ctx || ctx->isClosed() || !ctx->isSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->setUserNonBlock(arg & O_NONBLOCK);//user set nonblock
            if (ctx->getSysNonBlock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        } 
        break;
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
    case F_SETPIPE_SZ:
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
    case F_GETFL:
        {   
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            tao::FdCtx::ptr ctx = tao::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                return arg;
            }
            if(ctx->getSysNonBlock()) {
                // if (!(arg & O_NONBLOCK)) {
                //     fcntl_f(fd, cmd, arg | O_NONBLOCK);
                // }
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
    case F_GETFD:
    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
    case F_GETPIPE_SZ:
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        break;
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK: 
        {
            struct flock* arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
    case F_GETOWN_EX:
    case F_SETOWN_EX:
        {
            struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
    default:
        va_end(va);
        return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if (FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        tao::FdCtx::ptr ctx  = tao::FdMgr::GetInstance()->get(d);
        if (!ctx || ctx->isClosed() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonBlock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}






}


