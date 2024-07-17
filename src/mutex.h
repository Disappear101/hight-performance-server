#ifndef __TAO_MUTEX_H__
#define __TAO_MUTEX_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdexcept>
#include <atomic>
#include <list>
#include "noncopyable.h"
#include "fiber.h"

namespace tao {

class Semaphore : Noncopyable {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    void notify();
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
        if (m_locked) {
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

class Mutex : Noncopyable {
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

class NullMutex : Noncopyable {
public:
    using Lock = ScopedLockImpl<NullMutex>;

    NullMutex() {}

    ~NullMutex() {}

    void lock() {}

    void unlock() {}
};

class RWMutex : Noncopyable {
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

class NullRWMutex : Noncopyable {
public:
    /// local read lock
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    /// local write lock
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    NullRWMutex() {}

    ~NullRWMutex() {}

    void readLock() {}

    void writeLock() {}

    void unlock() {}
};

class SpinLock : Noncopyable{
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
//Compare-And-Swap
//implements a spinlock using atomic operations, ensuring thread safety by forcing other 
//threads to spin-wait until the lock becomes available. This type of lock is very 
//efficient for short critical sections where the wait time is expected to be minimal. 
class CASLock {
public:
    CASLock() {
        m_mutex.clear();
    }
    ~CASLock() {

    }
    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    //atomic status
    volatile std::atomic_flag m_mutex;
};

class Scheduler;
class FiberSemaphore : Noncopyable {
public:
    using MutexType = SpinLock;

    FiberSemaphore(size_t initial_concurrency = 0);
    ~FiberSemaphore();

    bool tryWait();
    void wait();
    void notify();

    size_t getConcurrency() const { return m_concurrency;}
    void reset() { m_concurrency = 0;}
private:
    MutexType m_mutex;
    std::list<std::pair<Scheduler*, Fiber::ptr> > m_waiters;
    size_t m_concurrency;
};

}

#endif
