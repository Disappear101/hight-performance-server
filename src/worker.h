#ifndef __TAO_WORKER_H__
#define __TAO_WORKER_H__

#include "mutex.h"
#include "singleton.h"
#include "log.h"
#include "iomanager.h"

namespace tao {

class WorkerGroup : Noncopyable, public std::enable_shared_from_this<WorkerGroup> {
public:
    using ptr = std::shared_ptr<WorkerGroup>;
    static WorkerGroup::ptr Create(uint32_t batch_size, tao::Scheduler *s = tao::Scheduler::GetThis()) {
        return std::make_shared<WorkerGroup>(batch_size, s);
    }

    WorkerGroup(uint32_t batch_size, tao::Scheduler* s = tao::Scheduler::GetThis());
    ~WorkerGroup();

    void schedule(std::function<void()>cb, int thread = -1);
    void waitAll();
private:
    void doWork(std::function<void()>cb);
private:
    uint32_t m_batchSize;
    bool m_finish;
    Scheduler* m_scheduler;
    FiberSemaphore m_sem;
};

class WorkerManager {
public:
    WorkerManager();
    void add(Scheduler::ptr s);
    Scheduler::ptr get(const std::string& name);
    IOManager::ptr getAsIOManager(const std::string& name);

    template<class FiberOrCb>
    void schedule(const std::string& name, FiberOrCb fc, int thread) {
        auto s = get(name);
        if (s) {
            s->schedule(fc, thread);
        } else {
            static tao::Logger::ptr s_logger = TAO_LOG_NAME("system");
            TAO_LOG_ERROR(s_logger) << "schedule name=" << name
                << " not exists";
        }
    }

    //init configuration from conf file
    bool init();

    //init configuration from data structure
    bool init(const std::map<std::string, std::map<std::string, std::string>>&v);
    void stop();

    bool isStopped() const { return m_stop;}
    std::ostream& dump(std::ostream& os);

    uint32_t getCount();
private:
    //name -> Schdulers
    std::map<std::string, std::vector<Scheduler::ptr> > m_datas;
    bool m_stop;
};

using WorkerMgr = tao::Singleton<WorkerManager>;

}

#endif