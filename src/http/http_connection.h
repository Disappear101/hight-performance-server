#ifndef __TAO_HTTP_CONNECTION_H__
#define __TAO_HTTP_CONNECTION_H__

#include "../streams/socket_stream.h"
#include "uri.h"
#include "http.h"
#include <memory>

namespace tao {
namespace http {

struct HttpResult {
    using ptr = std::shared_ptr<HttpResult>;

    enum class Error {
        OK = 0,

        // invalid url
        INVALID_URL = 1,

        // unparsable host
        INVALID_HOST = 2,

        // connect failed
        CONNECT_FAIL = 3,

        // connection closed by peer
        SEND_CLOSE_BY_PEER = 4,

        // socket error when send request 
        SEND_SOCKET_ERROR = 5,

        // time out 
        TIMEOUT = 6,

        //create socket failed
        CREATE_SOCKET_ERROR = 7,

        //get connection failed from connection pool
        POOL_GET_CONNECTION = 8,

        //invalide connection
        POOL_INVALID_CONNECTION = 9
    };

    HttpResult(int _res, HttpResponse::ptr _rsp, const std::string& _err) 
        :result(_res)
        ,response(_rsp)
        ,error(_err){

    }

    int result;
    HttpResponse::ptr response;
    std::string error;
};

class HttpConnection : public SocketStream {
public:
    using ptr = std::shared_ptr<HttpConnection>;
    //HttpSession();
    HttpConnection(Socket::ptr sock, bool owner = true);
    ~HttpConnection() {}

    static HttpResult::ptr DoGet(const std::string& url
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = std::string());

    static HttpResult::ptr DoGet(Uri::ptr uri
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = "");

    static HttpResult::ptr DoPost(const std::string& url
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());

    static HttpResult::ptr DoPost(Uri::ptr uri
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());

    static HttpResult::ptr DoRequest(HttpMethod method
                                        , const std::string& url
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string>& headers = {}
                                        , const std::string& body = std::string());

    static HttpResult::ptr DoRequest(HttpMethod method
                                        , Uri::ptr uri
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string>& headers = {}
                                        , const std::string& body = std::string());

    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                            , Uri::ptr uri
                            , uint64_t timeout_ms);


    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr req);
};

}
}

#endif