#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include "stdint.h"

namespace sylar {

class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    void notify();

private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
private:
    sem_t m_semaphore;
};

template<class T>
struct ScopedLockImpl
{
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }
    ~ScopedLockImpl() {
        unlock();
    }
    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock() {
        if (!m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.readLock();
        m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.readLock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.writeLock();
        m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.writeLock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class Mutex {
public:
    using Lock = ScopedLockImpl<Mutex>;
    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
private:
};

class NullMutex {
public:
    using Lock = ScopedLockImpl<NullMutex>;

    NullMutex() {}

    ~NullMutex() {}

    void lock() {}

    void unlock() {}
};

class RWMutex {
public:
    using ReadLock =  ReadScopedLockImpl<RWMutex>;
    using WriteLock = WriteScopedLockImpl<RWMutex>;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }
    void readLock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    void writeLock() {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
}; 

class NullRWMutex {
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    NullRWMutex() {}

    ~NullRWMutex() {}

    void readLock() {}

    void writeLock() {}

    void unlock() {}
};

class SpinLock {
public:
    using Lock = ScopedLockImpl<SpinLock>;
    SpinLock() {
        pthread_spin_init(&m_mutex, 0);
    }
    ~SpinLock() {
        pthread_spin_destroy(&m_mutex);
    }
    void lock() {
        pthread_spin_lock(&m_mutex);
    }
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

//atomic lock
// class CASLock {
// public:
//     CASLock() {
//         m_mutex.clear();
//     }
//     ~CASLock() {

//     }
//     void lock() {
//         while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
//     }
//     void unlock() {
//         std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
//     }
// private:
//     //atomic status
//     volatile std::atomic_flag m_mutex;
// };

class Thread {
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
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    //Thread& operator=(const Thread&) =  delete;

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