#include "../src/address.h"
#include "../src/log.h"
#include <map>

tao::Logger::ptr g_logger = TAO_LOG_ROOT();

void test() {
    std::vector<tao::Address::ptr> addrs;

    if (!tao::Address::Lookup(addrs, "www.baidu.com:ftp")) {
        TAO_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        TAO_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<tao::Address::ptr, uint32_t> > results;

    bool v = tao::Address::GetInterfaceAddresses(results);
    if(!v) {
        TAO_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i : results) {
        TAO_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = tao::IPAddress::Create("www.sylar.top");
    auto addr = tao::IPAddress::Create("127.0.0.8");
    if(addr) {
        TAO_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv){
    //test();
    //test_iface();
    test_ipv4();
    return 0;
}