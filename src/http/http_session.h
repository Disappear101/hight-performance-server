#ifndef __TAO_HTTP_SESSION_H__
#define __TAO_HTTP_SESSION_H__

#include "../streams/socket_stream.h"
#include "http.h"
#include <memory>

namespace tao {
namespace http {

class HttpSession : public SocketStream {
public:
    using ptr = std::shared_ptr<HttpSession>;
    //HttpSession();
    HttpSession(Socket::ptr sock, bool owner = true);
    ~HttpSession() {}

    HttpRequest::ptr recvRequest();

    int sendResponse(HttpResponse::ptr rsp);
};

}
}

#endif