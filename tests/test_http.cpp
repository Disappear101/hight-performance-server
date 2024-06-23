#include "../src/http/http.h"
//#include "../src/log.h"

#include <memory>

void test_request() {
    tao::http::HttpRequest::ptr req = std::make_shared<tao::http::HttpRequest>();
    req->setHeader("host", "www.baidu.com");
    req->setBody("hellow world");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    tao::http::HttpResponse::ptr rsp = std::make_shared<tao::http::HttpResponse>();
    rsp->setHeader("X-X", "something");
    rsp->setBody("hello there");
    rsp->setStatus((tao::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}