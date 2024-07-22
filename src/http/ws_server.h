#ifndef __TAO_HTTP_WS_SERVER_H__
#define __TAO_HTTP_WS_SERVER_H__

#include "src/tcpserver.h"
#include "ws_session.h"
#include "ws_servlet.h" 

namespace tao {
namespace http {

class WSServer : public TcpServer {
public:
    using ptr = std::shared_ptr<WSServer>;

    WSServer(tao::IOManager* worker = tao::IOManager::GetThis()
             , tao::IOManager* accept_worker = tao::IOManager::GetThis());

    WSServletDispatch::ptr getWSServletDispatch() const { return m_dispatch;}
    void setWSServletDispatch(WSServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
protected:
    WSServletDispatch::ptr m_dispatch;
};

} // namespace http
} // namespace tao

#endif // __TAO_HTTP_WS_SERVER_H__
