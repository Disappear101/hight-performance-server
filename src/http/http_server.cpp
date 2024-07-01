#include "http_server.h"
#include "../log.h"

namespace tao {
namespace http {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

tao::http::HttpServer::HttpServer(bool keepalive, tao::IOManager *worker, tao::IOManager *accept_worker)
    :TcpServer(worker, accept_worker)
    ,m_isKeepalive(keepalive) 
    ,m_dispatch(std::make_shared<ServletDispatch>()){
}

void HttpServer::handleClient(tao::Socket::ptr client)
{
    //TAO_LOG_DEBUG(g_logger) << "handleClient " << *client;
    HttpSession::ptr session = std::make_shared<HttpSession>(client);
    do {
        auto req = session->recvRequest();
        if (!req) {
            TAO_LOG_WARN(g_logger) << "recv http request fail, errno = "
                    << errno << " errstr = " << strerror(errno)
                    << " client: " << *client;
            break;
        }
        HttpResponse::ptr rsp = std::make_shared<HttpResponse>(req->getVersion()
                            ,req->isClose() || !m_isKeepalive);
        rsp->setHeader("Server", getName());
        //rsp->setBody("hello there");
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);

        if (!m_isKeepalive || req->isClose()) {
            std::cout << "m_isKeepalive = " << m_isKeepalive
                    << "request status = " << req->isClose() << std::endl;
            break;
        }
    } while (true);
    session->close();
}
}


}


