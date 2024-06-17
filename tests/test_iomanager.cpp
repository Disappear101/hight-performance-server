#include "../src/iomanager.h"
#include "../src/log.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

int sock;

void test_fiber() {
    TAO_LOG_INFO(g_logger) << "test_fiber";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        TAO_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        tao::IOManager::GetThis()->addEvent(sock, tao::IOManager::READ, [](){
            TAO_LOG_INFO(g_logger) << "read callback";
        });
        tao::IOManager::GetThis()->addEvent(sock, tao::IOManager::WRITE, [](){
            TAO_LOG_INFO(g_logger) << "write callback";
            tao::IOManager::GetThis()->cancelEvent(sock, tao::IOManager::READ);
            close(sock);
        });
    } else {
        TAO_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test1() {
    tao::IOManager iom(2, true);
    iom.schedule(&test_fiber);
}

tao::Timer::ptr s_timer;
void test_timer() {
    tao::IOManager iom(2);

    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        TAO_LOG_INFO(g_logger) << "hello timer i=" << i;
        if (++i == 3) {
            //timer->cancle();
            s_timer->reset(2000, true);
        }
    }, true);
}

int main(int argc, char** argv) {
    //test1();
    test_timer();
    return 0;
}