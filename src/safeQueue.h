#ifndef __TAO__SATEQUEUE__
#define __TAO__SATEQUEUE__

#include <queue>
#include "mutex.h"

namespace tao{

template <typename T>
class SafeQueue
{
public:
    using MutexType = RWMutex;
    SafeQueue() {}
    SafeQueue(SafeQueue &&other) {}
    ~SafeQueue() {}

    bool empty()
    {
        MutexType::ReadLock lock(m_mutex);
        return m_queue.empty();
    }

    int size()
    {
        MutexType::ReadLock lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T &t)
    {
        MutexType::WriteLock lock(m_mutex);
        m_queue.emplace(t);
    }

    bool dequeue(T &t)
    {
        if (m_queue.empty())
            return false;

        {
            MutexType::ReadLock lock(m_mutex);
            t = std::move(m_queue.front()); 
        }
        
        MutexType::WriteLock lock(m_mutex);
        m_queue.pop(); 

        return true;
    }

private:
    std::queue<T> m_queue; 

    MutexType m_mutex; 
};

}
#endif