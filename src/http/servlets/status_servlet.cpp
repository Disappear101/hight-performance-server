#include "status_servlet.h"
#include <iostream>
#include <string>
#include <vector>

namespace tao {
namespace http {

StatusServlet::StatusServlet()
    :Servlet("Status Servlet"){
}

int32_t StatusServlet::handle(tao::http::HttpRequest::ptr request, tao::http::HttpResponse::ptr response, tao::http::HttpSession::ptr session)
{
    response->setHeader("Content-Type", "text/text; charset=utf-8");

#define XX(key) \
    ss << std::setw(30) << std::right << key ": "
    std::stringstream ss;
    ss << "===================================================" << std::endl;
    

    response->setBody(ss.str());

    return 0;
}
}
}