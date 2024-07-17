#include "application.h"

#include <unistd.h>
#include <signal.h>

#include "daemon.h"
#include "config.h"
#include "env.h"
#include "log.h"
#include "http/http_server.h"
#include "worker.h"

namespace tao {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

static tao::ConfigVar<std::string>::ptr g_server_work_path = 
    tao::Config::Lookup("server.work_path", std::string("/apps/work/tao"), "server work path");

static tao::ConfigVar<std::string>::ptr g_server_pid_file =
    tao::Config::Lookup("server.pid_file"
            ,std::string("tao.pid")
            , "server pid file");

static tao::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf = 
    tao::Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");

Application::Application()
{
    //s_instance = this;
}

bool Application::init(int argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;

    tao::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    tao::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    tao::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    tao::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;

    if (!tao::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if (tao::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    std::string conf_path = tao::EnvMgr::GetInstance()->getConfigPath();
    TAO_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    tao::Config::LoadFromConfDir(conf_path);

    if(is_print_help) {
        tao::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    int run_type = 0;
    if(tao::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(tao::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }
    if(run_type == 0) {
        tao::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    std::string pidfile = g_server_work_path->getValue() 
                            + "/" + g_server_pid_file->getValue();
    if (tao::FSUtil::IsRunningPidfile(pidfile)) {
        TAO_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    } 
    if(!tao::FSUtil::Mkdir(g_server_work_path->getValue())) {
        TAO_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}
bool Application::run()
{
    bool is_daemon = tao::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv, std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2), is_daemon);
}
bool Application::getSever(const std::string &type, std::vector<TcpServer::ptr> &svrs)
{
    return false;
}
void Application::listAllSever(std::map<std::string, std::vector<TcpServer::ptr>> &servers)
{
}
int Application::main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    TAO_LOG_INFO(g_logger) << "main";
    std::string conf_path = tao::EnvMgr::GetInstance()->getConfigPath();
    tao::Config::LoadFromConfDir(conf_path, true);
    {
        std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if(!ofs) {
            TAO_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }

    m_mainIOManager.reset(new tao::IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(2000, [](){
        //TAO_LOG_INFO(g_logger) << "hello";
    }, true);
    m_mainIOManager->stop();
    return 0;
}
int Application::run_fiber()
{
    tao::WorkerMgr::GetInstance()->init();

    auto http_confs = g_servers_conf->getValue();
    std::vector<TcpServer::ptr> svrs;
    for(auto& i : http_confs) {
        TAO_LOG_DEBUG(g_logger) << std::endl << Lexical_Cast<TcpServerConf, std::string>()(i);

        std::vector<Address::ptr> address;
        //parse address
        for(auto& a : i.address) {
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                //tao_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }
            int32_t port = atoi(a.substr(pos + 1).c_str());
            auto addr = tao::IPAddress::Create(a.substr(0, pos).c_str(), port);
            if (addr) {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t>> result;
            if (tao::Address::GetInterfaceAddresses(result, a.substr(0, pos))) {
                for (auto&x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if (ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }
            auto aaddr = tao::Address::LookupAny(a);
            if(aaddr) {
                address.push_back(aaddr);
                continue;
            }
            TAO_LOG_ERROR(g_logger) << "invalid address: " << a;
            _exit(0);
        }

        IOManager* accept_worker = tao::IOManager::GetThis();
        IOManager* process_worker = tao::IOManager::GetThis();
        if (!i.accept_worker.empty() && i.accept_worker != "caller") {
            accept_worker = tao::WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if (!accept_worker) {
                TAO_LOG_ERROR(g_logger) << "accept_worker: " << i.accept_worker
                    << " not exists";
                _exit(0);
            }
        }
        if (!i.process_worker.empty() && i.process_worker != "caller") {
            process_worker = tao::WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
            if (!process_worker) {
                TAO_LOG_ERROR(g_logger) << "process_worker: " << i.process_worker
                    << " not exists";
                _exit(0);
            }
        }

        TcpServer::ptr server;
        if (i.type == "http") {
            server.reset(new tao::http::HttpServer(i.keepalive, process_worker, accept_worker));
        } else if (i.type == "tcp") {
            server = std::make_shared<TcpServer>(process_worker, accept_worker);
        } else {
            TAO_LOG_ERROR(g_logger) << "invalid server type=" << i.type
            << Lexical_Cast<TcpServerConf, std::string>()(i);
            _exit(0);
        }

        if (!i.name.empty()) {
            server->setName(i.name);
        }

        std::vector<Address::ptr>fails;
        if (!server->bind(address, fails)) {
            for(auto& x : fails) {
                TAO_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
            }
            _exit(0);
        }

        server->setConf(i);
        m_servers[i.type].push_back(server);
        svrs.push_back(server);
    }

    for (auto& i : svrs) {
        i->start();
    }

    return 0;
}
}