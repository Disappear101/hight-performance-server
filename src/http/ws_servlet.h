#ifndef __HTTP_WS_SERVLET_H__
#define __HTTP_WS_SERVLET_H__

#include "src/http/servlet.h"
#include "src/http/ws_session.h"

namespace tao {
namespace http {
class WSServlet : public Servlet {
public:
    using ptr = std::shared_ptr<WSServlet>;
    WSServlet(const std::string& name)
        :Servlet(name) {
    }
    virtual ~WSServlet() {}

    virtual int32_t handle(tao::http::HttpRequest::ptr request
                   , tao::http::HttpResponse::ptr response
                   , tao::http::HttpSession::ptr session) override {
        return 0;
    }

    virtual int32_t onConnect(tao::http::HttpRequest::ptr header
                              ,tao::http::WSSession::ptr session) = 0;
    virtual int32_t onClose(tao::http::HttpRequest::ptr header
                             ,tao::http::WSSession::ptr session) = 0;
    virtual int32_t handle(tao::http::HttpRequest::ptr header
                           ,tao::http::WSFrameMessage::ptr msg
                           ,tao::http::WSSession::ptr session) = 0;
    const std::string& getName() const { return m_name;}
protected:
    std::string m_name;
};

class FunctionWSServlet : public WSServlet {
public:
    using ptr = std::shared_ptr<FunctionWSServlet>;
    using on_connect_cb = std::function<int32_t (tao::http::HttpRequest::ptr header
                              ,tao::http::WSSession::ptr session)>;
    using on_close_cb = std::function<int32_t (tao::http::HttpRequest::ptr header
                             ,tao::http::WSSession::ptr session)>; 
    using callback = std::function<int32_t (tao::http::HttpRequest::ptr header
                           ,tao::http::WSFrameMessage::ptr msg
                           ,tao::http::WSSession::ptr session)>;

    FunctionWSServlet(callback cb
                      ,on_connect_cb connect_cb = nullptr
                      ,on_close_cb close_cb = nullptr);

    virtual int32_t onConnect(tao::http::HttpRequest::ptr header
                              ,tao::http::WSSession::ptr session) override;
    virtual int32_t onClose(tao::http::HttpRequest::ptr header
                             ,tao::http::WSSession::ptr session) override;
    virtual int32_t handle(tao::http::HttpRequest::ptr header
                           ,tao::http::WSFrameMessage::ptr msg
                           ,tao::http::WSSession::ptr session) override;
protected:
    callback m_callback;
    on_connect_cb m_onConnect;
    on_close_cb m_onClose;
};

class WSServletDispatch : public ServletDispatch {
public:
    using ptr = std::shared_ptr<WSServletDispatch>;
    using RWMutexType = RWMutex;

    WSServletDispatch();
    void addServlet(const std::string& uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb = nullptr
                    ,FunctionWSServlet::on_close_cb close_cb = nullptr);
    void addGlobServlet(const std::string& uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb = nullptr
                    ,FunctionWSServlet::on_close_cb close_cb = nullptr);
    WSServlet::ptr getWSServlet(const std::string& uri);
};

}
}
#endif