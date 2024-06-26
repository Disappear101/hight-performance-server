# C/C++ High Performance Server

## 1. Log Modular

* **Support Multiple Information Field** 
  * %m: message 
  * %p:  log level 
  * %r:  elapse counter(ms) 
  * %c:  loger name 
  * %t:  thread id 
  * %n:  new line 
  * %d:  date time 
  * %f:  file name 
  * %l:  line number 
  * %T:  Tab 
  * %F:  coroutine id 
  * %N:  thread name 
* **Support Multiple Log Levels**
   ```
   enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
   ```
* **Support stdout stream and file stream**
* **Class Diagram**
 ![log system class diagram](https://github.com/Disappear101/hight-performance-server/assets/105203326/9d4a4e1e-837a-4a6b-95e4-6946495ee0d0)


## 2. Configure Modular
This system uses YAML for configuration file parsing and includes template classes for converting between strings and various data types. 
The implementation also supports dynamic configuration variable registration and retrieval. 
Configuration modular used STL for container generalization, boost library for lexical casting, YAML-CPP for YAML parsing and achieved
key features:
* **Dynamic Configuration Lookup**:
 The LookupBase method allows retrieving configuration variables by name.
 Template-based Lookup methods support retrieval with type safety.
* **YAML Integration**:
 The LoadFromYaml method loads configuration from a YAML node, processing all nodes and assigning values to the corresponding configuration variables.
* **Serialization and Deserialization**:
 The Lexical_Cast template classes handle conversions between strings and common data structures. Exploit Partial Specialization features
 to Generalize common STL(e.g., vectors, lists, sets, maps).
* **Callback Mechanism**:
 Configuration variables support change listeners, allowing actions to be triggered when values are updated.
* **Thread Safety**:
 The use of customized RWMutexType ensures thread-safe operations on configuration data.

Therefore, it can handle different data type, customized struct, container and Nested containers. 
For example:
```
system:
    int_vec: [10, 20, 100, 80]
    int_list: [10, 20, 70]
    int_set: [10, 20, 40]
    str_int_map:
        k: 20
        h: 50
        z: 80
class:
    person:
        name: Tom
        age: 52
        sex: true
    map:
        Sachin:
            name: Sachin
            age: 18
            sex: true
        Yahya:
            name: Yahya
            age: 16
            sex: true
    vec_map:
        farmer:
            - name: Jacky
              age: 33
              sex: true
            - name: Jacy
              age: 25
              sex: false
        Bauer:
            - name: b1
              age: 29
              sex: true
            - name: b2
              age: 31
              sex: false
```

## 3. Thread and Mutex
Here encapsulated semaphore and several types of locks, each suited for different scenarios in concurrent programming. 
* **semaphore**
 A semaphore is a synchronization primitive used to control access to a common resource in a concurrent system such as a multiprogramming operating system.
 Semaphores can be used to solve various synchronization problems like controlling access to a pool of resources or ensuring that certain sequences of operations are performed in the correct order.
* **Mutex**
 This is a standard mutex based on POSIX threads (pthreads). It ensures mutual exclusion, allowing only one thread to access the critical section at a time.
 Use this when you need simple, straightforward mutual exclusion without reader/writer differentiation. Suitable for protecting small critical sections and avoiding race conditions.
* **RWMutex**
 A read-write mutex that allows multiple concurrent readers or one writer. It uses POSIX read-write locks. Ideal for situations where reads are more frequent than writes.
 It improves performance by allowing concurrent read access while still ensuring exclusive access for writes.
* **SpinLock**
 A spinlock that uses busy-waiting instead of blocking. Threads attempting to acquire the lock will continuously check until the lock becomes available.
 Suitable for very short critical sections where the overhead of blocking and waking up threads would be higher than spinning. Not recommended for long critical sections due to potential CPU waste.
* **CASLock**
 A lock that uses the Compare-And-Swap (CAS) atomic operation to implement a spinlock. This ensures thread safety by spinning until the lock is acquired.
 Efficient for short critical sections, similar to SpinLock, but potentially more efficient due to the atomic operations used. It avoids the overhead of system calls in some cases.

Thread class offers a robust implementation for managing threads in a C++ application. 
It encapsulates the details of thread creation, management, worker function, and synchronization, providing a higher-level interface for multithreading.
Utilizes a semaphore to ensure proper thread initialization before proceeding.

For test case: two threads write a file at the same time by using different locks.
The experiment measured the write speed and CPU usage of different types of mutexes used for thread synchronization. 
| Mutex Type | write speed | CPU usage |
|----------|----------|----------|
| NonMutex | 20 M/s | high |
| Mutex | 5.5 M/s | low |
| Spinlock | 7 M/s | high |
| CASlock | 8 M/s | high |

The experiment shows that while NonMutex provides the highest write speed at the cost of high CPU usage and thread safety, 
Mutex offers lower write speed with efficient CPU usage, whereas Spinlock and CASlock offer moderate write speeds with high CPU usage, suitable for short critical sections.

## 4. Coroutines

## 5. Coroutines Scheduler

## 6. Timer

## 7. IOManager

## 8. Hook

## 9. ByteArray

## 10.Http



