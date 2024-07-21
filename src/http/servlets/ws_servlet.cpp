#include "ws_servlet.h"

namespace tao {
namespace http {

FunctionWSServlet::FunctionWSServlet(callback cb
                                     ,on_connect_cb connect_cb
                                     ,on_close_cb close_cb)
    :WSServlet("FunctionWSServlet")
    ,m_callback(cb)
    ,m_onConnect(connect_cb)
    ,m_onClose(close_cb) {
}

int32_t FunctionWSServlet::onConnect(tao::http::HttpRequest::ptr header
                                     ,tao::http::WSSession::ptr session) {
    if(m_onConnect) {
        return m_onConnect(header, session);
    }
    return 0;
}

int32_t FunctionWSServlet::onClose(tao::http::HttpRequest::ptr header
                                     ,tao::http::WSSession::ptr session) {
    if(m_onClose) {
        return m_onClose(header, session);
    }
    return 0;
}

int32_t FunctionWSServlet::handle(tao::http::HttpRequest::ptr header
                                   ,tao::http::WSFrameMessage::ptr msg
                                   ,tao::http::WSSession::ptr session) {
    if(m_callback) {
        return m_callback(header, msg, session);
    }
    return 0;
}

WSServletDispatch::WSServletDispatch() {
    m_name = "WSServletDispatch";
}

void WSServletDispatch::addServlet(const std::string& uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb
                    ,FunctionWSServlet::on_close_cb close_cb) {
    ServletDispatch::addServlet(uri, std::make_shared<FunctionWSServlet>(cb, connect_cb, close_cb));
}

void WSServletDispatch::addGlobServlet(const std::string& uri
                    ,FunctionWSServlet::callback cb
                    ,FunctionWSServlet::on_connect_cb connect_cb
                    ,FunctionWSServlet::on_close_cb close_cb) {
    ServletDispatch::addGlobServlet(uri, std::make_shared<FunctionWSServlet>(cb, connect_cb, close_cb));
}

WSServlet::ptr WSServletDispatch::getWSServlet(const std::string& uri) {
    auto slt = getMatchedServlet(uri);
    return std::dynamic_pointer_cast<WSServlet>(slt);
}

}
}
