#include "module.h"

namespace tao {

void Module::onBeforeArgsParse(int argc, char **argv)
{
}

void Module::onAfterArgsParse(int argc, char **argv)
{
}

bool Module::onLoad()
{
    return false;
}

bool Module::onUnload()
{
    return false;
}

bool Module::onConnect(tao::Stream::ptr stream)
{
    return false;
}

bool Module::onDisconnect(tao::Stream::ptr stream)
{
    return false;
}

bool Module::onServerReady()
{
    return false;
}

bool Module::onServerUp()
{
    return false;
}

bool Module::handleRequest(tao::Message::ptr req, tao::Message::ptr rsp, tao::Stream::ptr stream)
{
    return false;
}

bool Module::handleNotify(tao::Message::ptr notify, tao::Stream::ptr stream)
{
    return false;
}

std::string Module::statusString()
{
    return std::string();
}

void Module::registerService(const std::string &server_type, const std::string &domain, const std::string &service)
{
    
}

}