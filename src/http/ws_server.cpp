#include "ws_server.h"
#include "src/log.h"

namespace tao {
namespace http {
    
static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

WSServer::WSServer(tao::IOManager *worker, tao::IOManager *accept_worker)
    :TcpServer(worker, accept_worker)
    ,m_dispatch(std::make_shared<WSServletDispatch>()){
    m_type = "ws";
}

void WSServer::handleClient(Socket::ptr client)
{
    TAO_LOG_DEBUG(g_logger) << "handleClient " << *client;
    WSSession::ptr session(new WSSession(client));
    do {
        HttpRequest::ptr header = session->handleShake();
        if(!header) {
            TAO_LOG_DEBUG(g_logger) << "handleShake error";
            break;
        }
        WSServlet::ptr servlet = m_dispatch->getWSServlet(header->getPath());
        if(!servlet) {
            TAO_LOG_DEBUG(g_logger) << "no match WSServlet";
            break;
        }
        int rt = servlet->onConnect(header, session);
        if(rt) {
            TAO_LOG_DEBUG(g_logger) << "onConnect return " << rt;
            break;
        }
        while(true) {
            auto msg = session->recvMessage();
            if(!msg) {
                break;
            }
            rt = servlet->handle(header, msg, session);
            if(rt) {
                TAO_LOG_DEBUG(g_logger) << "handle return " << rt;
                break;
            }
        }
        servlet->onClose(header, session);
    } while(0);
    session->close();
}
}
}