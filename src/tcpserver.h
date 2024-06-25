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
    virtual bool stop();

    uint64_t getReadTimeout() const { return m_readTimeout;}
    std::string getName() const { return m_name;};
    void setReadTimeout(uint64_t v) { m_readTimeout = v;}
    void setName(const std::string& v) { m_name = v;}

    bool isStop() const { return m_isStop;};

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

private:
    std::vector<Socket::ptr> m_socks;       //sockets connected to server
    IOManager* m_worker;                    //(N thread : M fiber)handle io events
    IOManager* m_acceptWorker;              //accept worker
    uint64_t m_readTimeout;                 //didn't receive message for a long time 
    std::string m_name;                     //name
    bool m_isStop;

};

}
#endif