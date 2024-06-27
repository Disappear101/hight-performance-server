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

    auto sd = server->getServletDispatch();
    sd->addServlet("/tao/xx", [](tao::http::HttpRequest::ptr req
                ,tao::http::HttpResponse::ptr rsp
                ,tao::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/tao/*", [](tao::http::HttpRequest::ptr req
                ,tao::http::HttpResponse::ptr rsp
                ,tao::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}