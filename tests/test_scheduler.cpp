#include "../src/scheduler.h"
#include "../src/fiber.h"
#include "../src/thread.h"
#include "../src/log.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    TAO_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        tao::Scheduler::GetThis()->schedule(&test_fiber, tao::GetThreadId());
    }
}

int main(int argc, char** argv) {
    TAO_LOG_INFO(g_logger) << "main";
    tao::Scheduler sc(3, false, "test");
    sc.start();
    sleep(2);
    TAO_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    TAO_LOG_INFO(g_logger) << "over";
    return 0;
}
