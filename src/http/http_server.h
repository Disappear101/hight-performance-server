#ifndef __TAO_HTTP_SERVER_H__
#define __TAO_HTTP_SERVER_H__

#include "http.h"
#include "http_session.h"
#include "../tcpserver.h"

namespace tao {
namespace http {

class HttpServer : public TcpServer {
public:
    using ptr = std::shared_ptr<HttpServer>;
    HttpServer(bool keepalive = false
                ,tao::IOManager* worker = tao::IOManager::GetThis()
                ,tao::IOManager* accept_worker = tao::IOManager::GetThis());
protected:
    virtual void handleClient(tao::Socket::ptr client) override;

private:
    bool m_isKeepalive;
};

} 

}
#endif