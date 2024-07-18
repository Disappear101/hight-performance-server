#include "my_module.h"
#include "src/config.h"
#include "src/log.h"

namespace name_space {

static tao::Logger::ptr g_logger = TAO_LOG_ROOT();

MyModule::MyModule()
    :tao::Module("project_name", "1.0", "") {
}

bool MyModule::onLoad() {
    TAO_LOG_INFO(g_logger) << "onLoad";
    return true;
}

bool MyModule::onUnload() {
    TAO_LOG_INFO(g_logger) << "onUnload";
    return true;
}

bool MyModule::onServerReady() {
    TAO_LOG_INFO(g_logger) << "onServerReady";
    return true;
}

bool MyModule::onServerUp() {
    TAO_LOG_INFO(g_logger) << "onServerUp";
    return true;
}

}

extern "C" {

tao::Module* CreateModule() {
    tao::Module* module = new name_space::MyModule;
    TAO_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    return module;
}

void DestoryModule(tao::Module* module) {
    TAO_LOG_INFO(name_space::g_logger) << "CreateModule " << module;
    delete module;
}

}
