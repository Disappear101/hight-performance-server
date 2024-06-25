#include "../src/tcpserver.h"
#include "../src/log.h"
#include "../src/iomanager.h"
#include "../src/bytearray.h"
#include "../src/address.h"

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

class EchoServer : public tao::TcpServer
{
private:
    int m_type = 0;
public:
    EchoServer(int type);
    void handleClient(tao::Socket::ptr client) override;
};

EchoServer::EchoServer(int type)
    :m_type(type) {

}


void EchoServer::handleClient(tao::Socket::ptr client) {
    TAO_LOG_INFO(g_logger) << "Handle client " << *client;
    tao::ByteArray::ptr ba = std::make_shared<tao::ByteArray>();
    while (true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWritableBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            TAO_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            TAO_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1) {//text 
            std::cout << ba->toString();// << std::endl;
        } else {
            std::cout << ba->toHexString();// << std::endl;
        }
        std::cout.flush();
    }

}

int type = 1;

void run() {
    TAO_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = tao::Address::LookupAny("0.0.0.0:8888");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main(int argc, char** argv) {
    if(argc < 2) {
        TAO_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if(!strcmp(argv[1], "-b")) {
        type = 2;
    }

    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}


