#ifndef __TAO_THREAD_H__
#define __TAO_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include "stdint.h"
#include "noncopyable.h"
#include "mutex.h"

namespace tao {

class Thread : Noncopyable {
public:
    using ptr = std::shared_ptr<Thread>;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id;}
    const std::string& getName() const { return m_name;}

    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);
// private:
//     Thread(const Thread&) = delete;
//     Thread(const Thread&&) = delete;
//     //Thread& operator=(const Thread&) =  delete;

    static void* run(void* arg);
private:
    pid_t m_id = -1;
    pthread_t m_thread;
    std::function<void()> m_cb;
    std::string m_name;

    Semaphore m_semaphore;//default cout is 0
};


}

#endif