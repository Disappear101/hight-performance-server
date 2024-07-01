#include "tcpserver.h"
#include "config.h"

namespace tao {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

static tao::ConfigVar<uint64_t>::ptr g_tcpserver_read_timeout = 
    tao::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2));

TcpServer::TcpServer(tao::IOManager *worker
                        , tao::IOManager* accept_worker)
    :m_worker(worker)
    ,m_acceptWorker(accept_worker)
    ,m_readTimeout(g_tcpserver_read_timeout->getValue())
    ,m_name("tao/1.0.0")
    ,m_isStop(true){
}

TcpServer::~TcpServer()
{
    for (auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}
bool TcpServer::bind(tao::Address::ptr addr)
{
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}
bool TcpServer::bind(const std::vector<Address::ptr> &addrs
                        ,std::vector<Address::ptr>& fails)
{
    for (auto& i : addrs) {
        Socket::ptr sock = Socket::CreateTCP(i);
        if (!sock->bind(i)) {
            TAO_LOG_ERROR(g_logger) << "bind fail errno = "
                << errno << " errstr = " << strerror(errno)
                << " addr = [" << i->toString() << "]";
            fails.push_back(i);
            continue;
        }
        if (!sock->listen()) {
            TAO_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << i->toString() << "]";
            fails.push_back(i);
            continue;
        }
        m_socks.push_back(sock);
    }
    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& i : m_socks) {
        TAO_LOG_INFO(g_logger) << "type=" << m_type
            << " name=" << m_name
            << " server bind success: " << *i;
    }
    return true;
}
bool TcpServer::start()
{
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for (auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                    shared_from_this(), sock));
    }
    return true;
}
bool TcpServer::stop()
{
    return false;
}
void TcpServer::handleClient(tao::Socket::ptr client)
{
    TAO_LOG_INFO(g_logger) << "handleClient: " << *client;
}
void TcpServer::startAccept(tao::Socket::ptr sock)
{
    while (!m_isStop) {
        Socket::ptr client = sock->accept();
        if (client) {
            client->setRecvTimeout(m_readTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        } else {
            TAO_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
}
}