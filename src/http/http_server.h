#ifndef __TAO_HTTP_SERVER_H__
#define __TAO_HTTP_SERVER_H__

#include "http.h"
#include "http_session.h"
#include "../tcpserver.h"
#include "servlet.h"

namespace tao {
namespace http {

class HttpServer : public TcpServer {
public:
    using ptr = std::shared_ptr<HttpServer>;

    /**
     * @brief constructor
     * @param[in] keepalive long connection
     * @param[in] worker working scheduler
     * @param[in] accept_worker accept scheduler
     */
    HttpServer(bool keepalive = false
                ,tao::IOManager* worker = tao::IOManager::GetThis()
                ,tao::IOManager* accept_worker = tao::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

protected:
    virtual void handleClient(tao::Socket::ptr client) override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};

} 

}
#endif