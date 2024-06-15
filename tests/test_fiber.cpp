#include "../src/log.h"
#include "../src/fiber.h"
#include "../src/thread.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void run_in_fiber() {
    TAO_LOG_INFO(g_logger) << "run_in_fiber begin";
    tao::Fiber::GetThis()->YieldToHold();
    TAO_LOG_INFO(g_logger) << "run_in_fiber end";
    tao::Fiber::GetThis()->YieldToHold();
}

void test_fiber() {
    TAO_LOG_INFO(g_logger) << "main begin";
    {
        tao::Fiber::GetThis();
        tao::Fiber::ptr fiber = std::make_shared<tao::Fiber>(run_in_fiber);
        fiber->swapIn();//print "run_in_fiber begin"
        TAO_LOG_INFO(g_logger) << "scope after swapIn";
        fiber->swapIn();//print "run_in_fiber end"
        TAO_LOG_INFO(g_logger) << "scope after end";
        fiber->swapIn();//after fiber task end, it can not return back to main thread
    }
    TAO_LOG_INFO(g_logger) << "main end";
}

int main (int argc, char** argv) {
    tao::Thread::SetName("main");

    std::vector<tao::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(std::make_shared<tao::Thread>(&test_fiber, "name_" + std::to_string(i)));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}