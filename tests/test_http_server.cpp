#include "../src/http/http_server.h"
#include "../src/log.h"

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void run() {
    tao::http::HttpServer::ptr server = std::make_shared<tao::http::HttpServer>();
    tao::Address::ptr addr = tao::Address::LookupAnyIPAddress("0.0.0.0:8888");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    server->start();
}

int main(int argc, char** argv) {
    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}