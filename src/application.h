#ifndef __TAO_APPLICATION_H__
#define __TAO_APPLICATION_H__

#include "tcpserver.h"

namespace tao {

class Application {
public:
    Application();

    //static Application* GetInstance() { return s_instance;}

    bool init(int argc, char** argv);

    bool run();

    bool getSever(const std::string& type, std::vector<TcpServer::ptr>& svrs);
    void listAllSever(std::map<std::string, std::vector<TcpServer::ptr>>& servers);

private:
    int main(int argc, char** argv);
    int run_fiber();
private:
    int m_argc = 0;
    char** m_argv = nullptr;

    std::map<std::string, std::vector<TcpServer::ptr> > m_servers;
    IOManager::ptr m_mainIOManager;
    //static Application* s_instance;
};

}

#endif