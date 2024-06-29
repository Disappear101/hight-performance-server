#include "http_connection.h"
#include "http_parser.h"
#include "../log.h"
#include "../util.h"

namespace tao {
namespace http {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {

}

HttpResult::ptr HttpConnection::DoGet(const std::string &url
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string> &headers
                                        , const std::string &body)
{
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
    }
    return DoGet(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string> &headers
                                        , const std::string &body)
{
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string &url
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string> &headers
                                        , const std::string &body)
{
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }
    return DoPost(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri
                                        , uint64_t timeout_ms
                                        , const std::map<std::string, std::string> &headers
                                        , const std::string &body)
{
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                                            , const std::string &url
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers
                                            , const std::string &body)
{
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL
                , nullptr, "invalid url: " + url);
    }
    return DoRequest(method, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
                                            , Uri::ptr uri
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers, const std::string &body)
{
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    bool has_host = false;
    for (auto& i : headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) {
            if (strcasecmp(i.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }
        if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }
        req->setHeader(i.first, i.second);
    }

    if (!has_host) {
        req->setHeader("host", uri->getHost());
    }
    req->setBody(body);
    return DoRequest(req, uri, timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req
                                            , Uri::ptr uri
                                            , uint64_t timeout_ms)
{
    //get addr from uri
    Address::ptr addr = uri->createAddress();
    if (!addr) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST
                , nullptr, "invalid host: " + uri->getHost()); 
    }
    //create socket
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR
                , nullptr, "create socket fail: " + addr->toString()
                        + " errno=" + std::to_string(errno)
                        + " errstr=" + std::string(strerror(errno)));
    }
    //connect
    if(!sock->connect(addr)) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL
                , nullptr, "connect fail: " + addr->toString());
    }
    //send req
    sock->setRecvTimeout(timeout_ms);
    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if(rt == 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                , nullptr, "send request closed by peer: " + addr->toString());
    }
    if(rt < 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                    , nullptr, "send request socket error errno=" + std::to_string(errno)
                    + " errstr=" + std::string(strerror(errno)));
    }
    //handle response
    auto rsp = conn->recvResponse();
    if(!rsp) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                    , nullptr, "recv response timeout: " + addr->toString()
                    + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

HttpResponse::ptr HttpConnection::recvResponse()
{
    HttpResponseParser::ptr parser = std::make_shared<HttpResponseParser>();
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(
            new char[buff_size + 1], [](char* ptr){
                delete[] ptr;
            });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, buff_size - offset);
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if(offset == (int)buff_size) {
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    auto& client_parser = parser->getParser();
    std::string body;
    if(client_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if(!begin || len == 0) {
                    int rt = read(data + len, buff_size - len);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if(len == (int)buff_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while(!parser->isFinished());
            //len -= 2;
            
            TAO_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
            if(client_parser.content_len + 2 <= len) {
                body.append(data, client_parser.content_len);
                memmove(data, data + client_parser.content_len + 2
                        , len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            } else {
                body.append(data, len);
                int left = client_parser.content_len - len + 2;
                while(left > 0) {
                    int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while(!client_parser.chunks_done);
    } else {
        int64_t length = parser->getContentLength();
        if(length > 0) {
            body.resize(length);

            int len = 0;
            if(length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            } else {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if(length > 0) {
                if(readFixSize(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    if(!body.empty()) {
        auto content_encoding = parser->getData()->getHeader("content-encoding");
        TAO_LOG_DEBUG(g_logger) << "content_encoding: " << content_encoding
            << " size=" << body.size();
        // if(strcasecmp(content_encoding.c_str(), "gzip") == 0) {
        //     auto zs = ZlibStream::CreateGzip(false);
        //     zs->write(body.c_str(), body.size());
        //     zs->flush();
        //     zs->getResult().swap(body);
        // } else if(strcasecmp(content_encoding.c_str(), "deflate") == 0) {
        //     auto zs = ZlibStream::CreateDeflate(false);
        //     zs->write(body.c_str(), body.size());
        //     zs->flush();
        //     zs->getResult().swap(body);
        // }
        parser->getData()->setBody(body);
    }
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req)
{
    std::stringstream ss;
    ss << *req;
    std::string data = ss.str();
    //std::cout << ss.str() << std::endl;
    return writeFixSize(data.c_str(), data.size());
}
HttpConnectionPool::HttpConnectionPool(const std::string &host, const std::string &vhost, uint32_t port, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request)
    :m_host(host)
    ,m_vhost(vhost)
    ,m_port(port)
    ,m_maxSize(max_size)
    ,m_maxAliveTime(max_alive_time)
    ,m_maxRequest(max_request) {
}
HttpConnection::ptr HttpConnectionPool::getConnection()
{
    uint64_t now = tao::GetCurrentMS();
    std::vector<HttpConnection*> invalid_conns;
    HttpConnection* ptr = nullptr;
    MutexType::Lock lock(m_mutex);
    while (!m_conns.empty()) {
        auto conn = *m_conns.begin();
        m_conns.pop_front();
        if (!conn->isConnected()) {
            invalid_conns.push_back(conn);
            continue;
        }
        if (conn->m_createTime + m_maxAliveTime > now) {
            invalid_conns.push_back(conn);
            continue;
        }
        ptr = conn;
        break;
    }
    lock.unlock();
    for (auto i : invalid_conns) {
        delete i;
    }
    if (ptr) {
        return std::make_shared<HttpConnection>(*ptr);
    }
    return HttpConnection::ptr();
}
HttpResult::ptr HttpConnectionPool::doGet(const std::string &url
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers
                                            , const std::string &body)
{
    return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}
HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers
                                            , const std::string &body)
{
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doGet(ss.str(), timeout_ms, headers, body);
}
HttpResult::ptr HttpConnectionPool::doPost(const std::string &url
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers
                                            , const std::string &body)
{
    return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}
HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri
                                            , uint64_t timeout_ms
                                            , const std::map<std::string, std::string> &headers
                                            , const std::string &body)
{
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doPost(ss.str(), timeout_ms, headers, body);
}
HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
                                                , const std::string &url
                                                , uint64_t timeout_ms
                                                , const std::map<std::string, std::string> &headers
                                                , const std::string &body)
{
     HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(url);
    req->setMethod(method);
    req->setClose(false);
    bool has_host = false;
    for(auto& i : headers) {
        if(strcasecmp(i.first.c_str(), "connection") == 0) {
            if(strcasecmp(i.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }

        if(!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }

        req->setHeader(i.first, i.second);
    }
    if(!has_host) {
        if(m_vhost.empty()) {
            req->setHeader("Host", m_host);
        } else {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);
    return doRequest(req, timeout_ms);
}
HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
                                                , Uri::ptr uri
                                                , uint64_t timeout_ms
                                                , const std::map<std::string, std::string> &headers
                                                , const std::string &body)
{
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doRequest(method, ss.str(), timeout_ms, headers, body);
}
HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req
                                                , uint64_t timeout_ms)
{
    auto conn = getConnection();
    if(!conn) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_GET_CONNECTION
                , nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    auto sock = conn->getSocket();
    if(!sock) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_INVALID_CONNECTION
                , nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    sock->setRecvTimeout(timeout_ms);
    int rt = conn->sendRequest(req);
    if(rt == 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER
                , nullptr, "send request closed by peer: " + sock->getRemoteAddress()->toString());
    }
    if(rt < 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR
                    , nullptr, "send request socket error errno=" + std::to_string(errno)
                    + " errstr=" + std::string(strerror(errno)));
    }
    auto rsp = conn->recvResponse();
    if(!rsp) {
        return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT
                    , nullptr, "recv response timeout: " + sock->getRemoteAddress()->toString()
                    + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}
}
}