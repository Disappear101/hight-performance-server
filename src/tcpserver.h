#ifndef __TAO_TCPSERVER_H__
#define __TAO_TCPSERVER_H__

#include <memory>
#include <functional>
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"
#include "config.h"

namespace tao {

struct TcpServerConf {
    using ptr = std::shared_ptr<TcpServerConf>;

    std::vector<std::string> address;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    std::string id;
    std::string type = "http";
    std::string name;
    std::string accept_worker;
    std::string process_worker;
    std::map<std::string, std::string> args;

    bool isValid() const {
        return !address.empty();
    } 

    bool operator==(const TcpServerConf& oth) const {
        return address == oth.address
            && keepalive == oth.keepalive
            && timeout == oth.timeout
            && name == oth.name
            && accept_worker == oth.accept_worker
            && process_worker == oth.process_worker
            && args == oth.args
            && id == oth.id
            && type == oth.type;
    } 
};

template<>
class Lexical_Cast<std::string, TcpServerConf> {
public:
    TcpServerConf operator()(const std::string&v) {
        YAML::Node node = YAML::Load(v);
        TcpServerConf conf;
        conf.id = node["id"].as<std::string>(conf.id);
        conf.type = node["type"].as<std::string>(conf.type);
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        conf.accept_worker = node["accept_worker"].as<std::string>();
        conf.process_worker = node["process_worker"].as<std::string>();
        conf.args = Lexical_Cast<std::string
            ,std::map<std::string, std::string> >()(node["args"].as<std::string>(""));
        if (node["address"].IsDefined()) {
            for (size_t i = 0; i < node["address"].size(); ++i) {
                conf.address.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

template<>
class Lexical_Cast<TcpServerConf, std::string> {
public:
    std::string operator()(const TcpServerConf& conf) {
        YAML::Node node;
        node["id"] = conf.id;
        node["type"] = conf.type;
        node["name"] = conf.name;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        node["accept_worker"] = conf.accept_worker;
        node["process_worker"] = conf.process_worker;
        node["args"] = YAML::Load(Lexical_Cast<std::map<std::string, std::string>, std::string>()(conf.args));
        for(auto& i : conf.address) {
            node["address"].push_back(i);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

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

    TcpServerConf::ptr getConf() const { return m_conf;}
    void setConf(TcpServerConf::ptr v) { m_conf = v;}
    void setConf(const TcpServerConf& v);
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
    TcpServerConf::ptr m_conf;              //tcpserver config 
};

}
#endif