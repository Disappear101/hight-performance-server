#include "../src/hook.h"
#include "../src/log.h"
#include "../src/iomanager.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <netdb.h>

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

void test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    struct hostent *host = gethostbyname("www.baidu.com");
    if (host == NULL || host->h_addr == NULL) {
        std::cerr << "Failed to resolve hostname" << std::endl;
        return;
    }
    bcopy((char *)host->h_addr, (char *)&addr.sin_addr.s_addr, host->h_length);
    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    TAO_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    TAO_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    TAO_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    TAO_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    TAO_LOG_INFO(g_logger) << buff;

}

int main(int argc, char** argv) {
    //test_sleep();
    //test_socket();
    tao::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}