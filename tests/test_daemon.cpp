#include "../src/daemon.h"
#include "../src/log.h"
#include "../src/iomanager.h"

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

tao::Timer::ptr timer;
int server_main(int argc, char** argv) {
    TAO_LOG_INFO(g_logger) << tao::ProcessInfoMgr::GetInstance()->toString();
    tao::IOManager iom(1);
    timer = iom.addTimer(1000, [](){
            TAO_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            if(++count > 5) {
                timer->cancel();
            }
    }, true);
    return 0;
}

int main(int argc, char** argv) {
    return tao::start_daemon(argc, argv, server_main, argc != 1);
}
