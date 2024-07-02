#ifndef __TAO_HTTP_STATUS_SERVLET_H__
#define __TAO_HTTP_STATUS_SERVLET_H__

#include "../servlet.h"

namespace tao {
namespace http {

class StatusServlet : public Servlet {
public:
    StatusServlet();
    virtual int32_t handle(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session) override;
};


}

}

#endif