#ifndef __TAO_FIBER_H__
#define __TAO_FIBER_H__

#include <ucontext.h>
#include <memory>
#include <functional>
#include <stdint.h>
#include "mutex.h"
#include "util.h"
#include "macro.h"

namespace tao {

//enable_shared_from_this provide pointer of current instance
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;

    enum State {
        INIT,   //initial status
        HOLD,   //hold and suspend status
        EXEC,   //executing status
        TERM,   //terminated status
        READY,  //ready status
        EXCEPT  //exception status
    };

private:
    Fiber();//disable default constructor

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    //reset task cb without reallocat and free memory
    void reset(std::function<void()> cb);

    void setState(State state) {m_state = state;}

    //Thread->main_fiber(scheduler) <---> sub_fiber
    
    //switches to current fiber and obtain executive right
    void swapIn();

    //give up executive 
    void swapOut();

    //obtain id of current executing fiber in current thread 
    uint64_t getId() const { return m_id;}

    State getState() const { return m_state; }

public:
    //set current executing fiber 
    static void SetThis(Fiber* f);
    //return current executing fiber 
    static Fiber::ptr GetThis();
    //The coroutine switches to the background, gives up executive right and set as ready status
    static void YieldToReady();
    //The coroutine switches to the background, set as hold status 
    static void YieldToHold();
    //total number if fibers
    static uint64_t nFibers();

    //get
    static uint64_t GetFiberId();

    static void MainFunc();
private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;
};

}

#endif