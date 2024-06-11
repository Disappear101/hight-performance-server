#ifndef __TAO_THREAD_POOL_H__
#define __TAO_THREAD_POOL_H__

#include <functional>
#include "safeQueue.h"
#include "noncopyable.h"
#include <future>
#include <thread>
#include <mutex>

namespace tao {
    
class ThreadPool : Noncopyable
{
private:
    class ThreadWorker 
    {
    private:
        int m_id; 

        ThreadPool *m_pool; 
    public:
        ThreadWorker(ThreadPool *pool, const int id) : m_pool(pool), m_id(id)
        {
        }

        void operator()()
        {
            std::function<void()> func; 

            bool dequeued; 

            while (!m_pool->m_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);

                    if (m_pool->m_queue.empty())
                    {
                        m_pool->m_conditional_lock.wait(lock); 
                    }

                    dequeued = m_pool->m_queue.dequeue(func);
                }

                if (dequeued)
                    func();
            }
        }
    };

    bool m_shutdown; 

    SafeQueue<std::function<void()>> m_queue; 

    std::vector<std::thread> m_threads; 
    std::mutex m_conditional_mutex; 

    std::condition_variable m_conditional_lock; 

public:
    ThreadPool(const int n_threads = 4)
        : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false)
    {
        
    }

    // Inits thread pool
    void init() {
        for (int i = 0; i < m_threads.size(); ++i)
        {
            m_threads.at(i) = std::thread(ThreadWorker(this, i)); 
        }
    }

    // Waits until threads finish their current task and shutdowns the pool
    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all(); 

        for (int i = 0; i < m_threads.size(); ++i)
        {
            if (m_threads.at(i).joinable()) 
            {
                m_threads.at(i).join(); 
            }
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // Create a function with bounded parameter ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); 

        // Encapsulate it into a shared pointer in order to be able to copy construct
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]()
        {
            (*task_ptr)();
        };

        m_queue.enqueue(warpper_func);

        m_conditional_lock.notify_one();

        return task_ptr->get_future();
    }
};
}

#endif