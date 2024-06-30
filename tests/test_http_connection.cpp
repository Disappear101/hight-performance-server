#include <iostream>
#include "../src/http/http_connection.h"
#include "../src/log.h"
#include "../src/iomanager.h"
#include "../src/http/http_parser.h"
#include <fstream>

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void test_pool() {
    tao::http::HttpConnectionPool::ptr pool = std::make_shared<tao::http::HttpConnectionPool>(
                "www.sylar.top", "", 80, 10, 1000 * 30, 5);

    tao::IOManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 300);
            TAO_LOG_INFO(g_logger) << r->toString();
    }, true);
}

void run() {
    tao::Address::ptr addr = tao::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr) {
        TAO_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    tao::Socket::ptr sock = tao::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        TAO_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    tao::http::HttpConnection::ptr conn = std::make_shared< tao::http::HttpConnection>(sock);
    tao::http::HttpRequest::ptr req = std::make_shared< tao::http::HttpRequest>();

    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    TAO_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        TAO_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    TAO_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    TAO_LOG_INFO(g_logger) << "=========================";

    auto r = tao::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    TAO_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    TAO_LOG_INFO(g_logger) << "=========================";
    test_pool();
}

int main(int argc, char** argv) {
    tao::IOManager iom(2);
    iom.schedule(run);
    return 0;
}