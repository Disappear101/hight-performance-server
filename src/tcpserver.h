#ifndef __TAO_TCPSERVER_H__
#define __TAO_TCPSERVER_H__

#include <memory>
#include <functional>
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"

namespace tao {

class TcpServer : public std::enable_shared_from_this<TcpServer> 
                    , Noncopyable {
public:
    using ptr = std::shared_ptr<TcpServer>;

    TcpServer(tao::IOManager* worker = tao::IOManager::GetThis()
                ,tao::IOManager* accept_worker = tao::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(tao::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, 
                        std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual void stop();

    uint64_t getReadTimeout() const { return m_recvTimeout;}
    std::string getName() const { return m_name;};
    void setReadTimeout(uint64_t v) { m_recvTimeout = v;}
    void setName(const std::string& v) { m_name = v;}

    bool isStop() const { return m_isStop;};

    //void stop();

protected:
    virtual void handleClient(tao::Socket::ptr client);
    virtual void startAccept(tao::Socket::ptr sock);

protected:
    std::vector<Socket::ptr> m_socks;       //sockets connected to server
    IOManager* m_worker;                    //(N thread : M fiber)handle io events
    IOManager* m_acceptWorker;              //accept worker
    uint64_t m_recvTimeout;                 //didn't receive message for a long time 
    std::string m_name;                     //name
    std::string m_type = "tcp";             //type
    bool m_isStop;

};

}
#endif