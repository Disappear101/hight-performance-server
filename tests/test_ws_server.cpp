#include "src/http/ws_server.h"
#include "src/log.h"

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void run() {
    tao::http::WSServer::ptr server(new tao::http::WSServer);
    tao::Address::ptr addr = tao::Address::LookupAnyIPAddress("0.0.0.0:8888");
    if(!addr) {
        TAO_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    auto fun = [](tao::http::HttpRequest::ptr header
                  ,tao::http::WSFrameMessage::ptr msg
                  ,tao::http::WSSession::ptr session) {
        session->sendMessage(msg);
        return 0;
    };

    server->getWSServletDispatch()->addServlet("/tao", fun);
    while(!server->bind(addr)) {
        TAO_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    server->start();
}

int main(int argc, char** argv) {
    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}