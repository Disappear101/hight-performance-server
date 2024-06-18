#include "../src/hook.h"
#include "../src/log.h"
#include "../src/iomanager.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void test_sleep() {
    tao::IOManager iom;
    iom.schedule([](){
        sleep(2);
        TAO_LOG_INFO(g_logger) << "sleep 2s";
    });

    iom.schedule([](){
        sleep(3);
        TAO_LOG_INFO(g_logger) << "sleep 3s";
    });

    TAO_LOG_INFO(g_logger) << "test sleep";
}

int main(int argc, char** argv) {
    test_sleep();
    return 0;
}