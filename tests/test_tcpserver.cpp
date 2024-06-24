#include "../src/tcpserver.h"
#include "../src/iomanager.h"
#include "../src/log.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void run() {
    auto addr = tao::Address::LookupAny("0.0.0.0:8033");
    auto addr2 = tao::UnixAddress::ptr(new tao::UnixAddress("/tmp/unix_addr2"));
    std::vector<tao::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);

    tao::TcpServer::ptr tcp_server(new tao::TcpServer);
    std::vector<tao::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    //TAO_LOG_INFO(g_logger) << *addr;
}

int main(int argc, char** argv) {
    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}