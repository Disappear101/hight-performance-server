#ifndef __TAO_HTTP_CONNECTION_H__
#define __TAO_HTTP_CONNECTION_H__

#include "../streams/socket_stream.h"
#include "uri.h"
#include "http.h"
#include <memory>
#include <list>

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

class HttpConnectionPool;

class HttpConnection : public SocketStream {
friend class HttpConnectionPool;
public:
    using ptr = std::shared_ptr<HttpConnection>;
    //HttpSession();
    HttpConnection(Socket::ptr sock, bool owner = true);
    ~HttpConnection() {}

    static HttpResult::ptr DoGet(const std::string& url
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = std::string());

    static HttpResult::ptr DoGet(Uri::ptr uris
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());

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
private:
    uint64_t m_createTime = 0;
    uint64_t m_request = 0;
};

class HttpConnectionPool {
public:
    using ptr = std::shared_ptr<HttpConnectionPool>;
    using MutexType = Mutex;

    HttpConnectionPool(const std::string& host
                        , const std::string& vhost
                        , uint32_t port
                        , uint32_t max_size
                        , uint32_t max_alive_time
                        , uint32_t max_request);

    //get one connection from connection pool
    HttpConnection::ptr getConnection();

    /**
     * @brief send HTTP GET request
     * @param[in] url url to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doGet(const std::string& url
                          , uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = std::string());


    /**
     * @brief send HTTP GET request
     * @param[in] uri url to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doGet(Uri::ptr uri
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());


    /**
     * @brief send HTTP Post request
     * @param[in] url url to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doPost(const std::string& url
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());

 

    /**
     * @brief send HTTP Post request
     * @param[in] uri uri to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doPost(Uri::ptr uri
                           , uint64_t timeout_ms
                           , const std::map<std::string, std::string>& headers = {}
                           , const std::string& body = std::string());


    /**
     * @brief send HTTP request with specified method
     * @param[in] method Http method
     * @param[in] url url to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doRequest(HttpMethod method
                            , const std::string& url
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = std::string());


    /**
     * @brief send HTTP request with specified method
     * @param[in] method Http method
     * @param[in] uri uri to request
     * @param[in] timeout_ms timeout (ms)
     * @param[in] headers HTTP headers
     * @param[in] body request body
     * @return return HttpResult
     */
    HttpResult::ptr doRequest(HttpMethod method
                            , Uri::ptr uri
                            , uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = std::string());


    /**
     * @brief send HTTP request with HttpRequest
     * @param[in] req Http Request
     * @param[in] timeout_ms timeout (ms)
     * @return return HttpResult
     */
    HttpResult::ptr doRequest(HttpRequest::ptr req
                            , uint64_t timeout_ms);

private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
private: 
    //host name
    std::string m_host;
    //
    std::string m_vhost;
    uint32_t m_port;

    //max number of long connection. 
    //when number of long connection exceed maxSize, create short connection
    uint32_t m_maxSize;
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<uint32_t> m_total = {0};
};


}
}

#endif