#include "../src/thread.h"
#include "../src/log.h"
#include "../src/config.h"
#include "../src/util.h"
#include <vector>

tao::Logger::ptr g_logger = TAO_LOG_ROOT();
tao::RWMutex s_mutex;
int count = 0;

void func1() {
    TAO_LOG_INFO(g_logger) << "name: " << tao::Thread::GetName()
                             << " this.name: " << tao::Thread::GetThis()->getName()
                             << " id: " << tao::GetThreadId()
                             << " this.id: " << tao::Thread::GetThis()->getId();
    for (int i = 0; i < 100000; ++i) {
        tao::RWMutex::WriteLock lock(s_mutex);
        ++count;
    }
}

void func2() {
    while(true) {
        TAO_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void func3() {
    while(true) {
        TAO_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc, char** argv) {
    TAO_LOG_INFO(g_logger) << "thread test begin";
    //YAML::Node root = YAML::LoadFile("/home/tao/projects/hight-performance-server/bin/conf/log2.yml");
    //tao::Config::LoadFromYaml(root);
    std::vector<tao::Thread::ptr> thrs;
    for(int i = 0; i < 5; ++i) {
        tao::Thread::ptr thr = std::make_shared<tao::Thread>(&func1, "name_" + std::to_string(i * 2));
        thrs.push_back(thr);
    }
    // for(int i = 0; i < 1; ++i) {
    //     tao::Thread::ptr thr(new tao::Thread(&func2, "name_" + std::to_string(i * 2)));
    //     tao::Thread::ptr thr2(new tao::Thread(&func3, "name_" + std::to_string(i * 2 + 1))); 
    //     thrs.push_back(thr);
    //     thrs.push_back(thr2);
    // }
    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }

    TAO_LOG_INFO(g_logger) << "thread test end";
    TAO_LOG_INFO(g_logger) << "count = " << count;
    return 0;
}