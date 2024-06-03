#include "../src/config.h"
#include "../src/log.h"
#include <yaml-cpp/yaml.h>
//#include "sylar/env.h"
#include <iostream>

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->toString();
    return 0;
}