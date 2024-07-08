#ifndef __TAO_HTTP_SERVLET_H__
#define __TAO_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../mutex.h"


namespace tao {
namespace http {

class Servlet {
public:
    using ptr = std::shared_ptr<Servlet>;

    Servlet(const std::string& name) 
        :m_name(name) {}
    virtual ~Servlet() {}

    virtual int32_t handle(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session) = 0;

    const std::string& getName() const { return m_name;}

protected:
    std::string m_name;
};

class NotFoundServlet;

class FunctionServlet : public Servlet {
public:
    using ptr = std::shared_ptr<FunctionServlet>;
    using callback = std::function<int32_t(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session)>;

    FunctionServlet(callback cb);
    virtual int32_t handle(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};

class ServletDispatch : public Servlet {
public:
    using ptr = std::shared_ptr<ServletDispatch>;
    using RWMutexType = RWMutex;

    ServletDispatch();

    virtual int32_t handle(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);
    
    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    void setDefault(Servlet::ptr v) { m_default = v;};

    Servlet::ptr getDefault() const { return m_default;}
    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    Servlet::ptr getMatchedServlet(const std::string& uri);

private:
    RWMutexType m_mutex;
    //precise uri(tao/xxx) -> servlet
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    //blurred uri(tao/*) -> servlet
    std::vector<std::pair<std::string, Servlet::ptr>>m_globs;
    //default
    Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet {
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
  
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(tao::http::HttpRequest::ptr request
                    , tao::http::HttpResponse::ptr response
                    , tao::http::HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};


}
}

#endif