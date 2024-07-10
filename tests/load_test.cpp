#include "../src/http/http_server.h"
#include "../src/log.h"

tao::Logger::ptr g_logger = TAO_LOG_ROOT();
tao::IOManager::ptr worker;

void run() {
    g_logger->setLevel(tao::LogLevel::INFO);
    tao::Address::ptr addr = tao::Address::LookupAnyIPAddress("0.0.0.0:8888");
    if(!addr) {
        TAO_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    tao::http::HttpServer::ptr http_server = std::make_shared<tao::http::HttpServer>(true, worker.get());
    while(!http_server->bind(addr)) {
        TAO_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    http_server->start();

}

int main(int argc, char** argv) {
    tao::IOManager iom(1);
    worker.reset(new tao::IOManager(4, false));
    iom.schedule(run);
    return 0;
}