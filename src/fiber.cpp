#include "fiber.h"
#include <atomic>
#include "log.h"
#include "scheduler.h"
#include "macro.h"
#include "config.h"

namespace tao {
static Logger::ptr g_logger = TAO_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count (0);

//current executing fiber
static thread_local Fiber* t_fiber = nullptr;
//main fiber
static thread_local Fiber::ptr t_thread_Fiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MallocStackAllocator {
    public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

//main fiber constructor without allocated stack space
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this); 

    if (getcontext(&m_ctx)) {
        TAO_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;

    TAO_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    :m_id(++s_fiber_id)
    ,m_cb(cb){
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        TAO_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    TAO_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

//constructor for sub fiber 
Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        TAO_ASSERT(m_state == TERM
                || m_state == INIT
                || m_state == EXCEPT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {//if it is main fiber
        TAO_ASSERT(!m_cb);
        TAO_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
    TAO_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id 
                            << " total=" << s_fiber_count;
}


void Fiber::reset(std::function<void()> cb) {
    TAO_ASSERT(m_stack);
    TAO_ASSERT(m_state == TERM 
            || m_state == INIT)

    m_cb = cb;
    if (getcontext(&m_ctx)) {
        TAO_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::swapIn() {
    SetThis(this);
    TAO_ASSERT(m_state != EXEC);
    m_state = EXEC;

    //switch with scheduler fiber
    if (swapcontext(&Scheduler::GetSchedulerFiber()->m_ctx, &m_ctx)) {
        TAO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut() { 
    SetThis(Scheduler::GetSchedulerFiber());

    //switch with scheduler fiber
    if (swapcontext(&m_ctx, &Scheduler::GetSchedulerFiber()->m_ctx)) {
        TAO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_thread_Fiber->m_ctx, &m_ctx)) {
        TAO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_thread_Fiber.get());
    if(swapcontext(&m_ctx, &t_thread_Fiber->m_ctx)) {
        TAO_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    //Fiber::ptr main_fiber = std::make_shared<Fiber>();
    Fiber::ptr main_fiber(new Fiber);//construct main fiber and set as executing fiber
    TAO_ASSERT(t_fiber == main_fiber.get());
    t_thread_Fiber = main_fiber;//set main fiber
    return t_fiber->shared_from_this();
}

    //The coroutine switches to the background, gives up executive right and set as ready status
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    TAO_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}
    //The coroutine switches to the background, set as hold status 
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    TAO_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}
    //total number if fibers
uint64_t Fiber::nFibers() {
    return s_fiber_count;
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();//share_pointer use_count ++
    TAO_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        TAO_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << tao::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        TAO_LOG_ERROR(g_logger) << "Fiber Except: ";
    }

    //to solve incomplete destruction
    auto raw_ptr = cur.get();
    cur.reset();//share_pointer use_count --
    //raw_ptr->swapOut();
    raw_ptr->swapOut();

    TAO_ASSERT2(false, "never reach");
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    TAO_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        TAO_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << tao::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        TAO_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << tao::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    TAO_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}




}