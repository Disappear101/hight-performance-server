#include "../src/thread.h"
#include "../src/log.h"
#include "../src/config.h"
#include "../src/util.h"
#include <vector>
#include <filesystem>


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

unsigned long getFileSize(const std::string& fileName) {
    std::ifstream file(fileName, std::ifstream::ate | std::ifstream::binary);
    return file.tellg();  // Returns the current position of the get pointer
}

void measureLogSpeed(const std::string& filePath, int interval) {
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "File does not exist: " << filePath << std::endl;
        return;
    }
    while (true){
        unsigned long initialSize = getFileSize(filePath);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        unsigned long finalSize = getFileSize(filePath);

        unsigned long bytesWritten = finalSize - initialSize;
        double bytesPerSecond = bytesWritten / static_cast<double>(interval);

        TAO_LOG_INFO(TAO_LOG_NAME("system")) << " [NullLock] " << "Bytes written per second: " << bytesPerSecond << std::endl;
        //std::cout << " [NullLock] " << "Bytes written per second: " << bytesPerSecond << std::endl;

        //double entrySize = 100.0;  // Average size of an entry in bytes, adjust as needed
        //double entriesPerSecond = bytesPerSecond / entrySize;

        //TAO_LOG_INFO(TAO_LOG_NAME("system")) << " [NullLock] " << "Entries per second under NullLock: " << entriesPerSecond << std::endl;
    }
}

int main(int argc, char** argv) {
    TAO_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/home/tao/projects/hight-performance-server/bin/conf/log2.yml");
    tao::Config::LoadFromYaml(root);
    TAO_LOG_INFO(TAO_LOG_NAME("system")) << "A Test";
    std::vector<tao::Thread::ptr> thrs;
    // for(int i = 0; i < 5; ++i) {
    //     tao::Thread::ptr thr = std::make_shared<tao::Thread>(&func1, "name_" + std::to_string(i * 2));
    //     thrs.push_back(thr);
    // }
    for(int i = 0; i < 1; ++i) {
        tao::Thread::ptr thr = std::make_shared<tao::Thread>(&func2, "name_" + std::to_string(i * 2));
        tao::Thread::ptr thr2 = std::make_shared<tao::Thread>(&func3, "name_" + std::to_string(i * 2 + 1)); 
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }
    std::string logFilePath = "threads_write.txt";  // Path to your log file
    int measurementInterval = 5;         // Measurement interval in seconds
    std::function<void()> warpper_func = [logFilePath, measurementInterval]() {
        measureLogSpeed(logFilePath, measurementInterval);
    };
    //auto boundFunc = std::bind(measureLogSpeed, logFilePath, measurementInterval);
    tao::Thread::ptr th3 = std::make_shared<tao::Thread>(warpper_func, "log_speed_measure_thread");
    thrs.push_back(th3);

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }

    TAO_LOG_INFO(g_logger) << "thread test end";
    TAO_LOG_INFO(g_logger) << "count = " << count;
    return 0;
}