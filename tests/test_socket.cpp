#include "../src/socket.h"
#include "../src/log.h"
#include "../src/iomanager.h"

static tao::Logger::ptr g_looger = TAO_LOG_ROOT();

void test_socket() {
    tao::IPAddress::ptr addr = tao::Address::LookupAnyIPAddress("www.google.com");
    if (addr) {
        TAO_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        TAO_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    addr->setPort(80);
    tao::Socket::ptr sock = tao::Socket::CreateTCP(addr);
    TAO_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if (!sock->connect(addr)) {
        TAO_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        TAO_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        TAO_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        TAO_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    TAO_LOG_INFO(g_looger) << buffs;
}

int main(int argc, char** argv) {
    tao::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}