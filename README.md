# C/C++ High Performance Server

## Performance Evaluation

| Type | Number of Master | Number of Worker | KeepAlive | Concurrency Level | average-QPS | Failed Reqeusts | CPU Utilization(Master) | CPU Utilizarion(Worker) | 
|----------|----------|----------|----------|----------|----------|----------|----------|----------|
| this | 1 | 4 | false | 200 |  22145 | 0 | 48% | 35% |
| Nginx | 1 | 4 | false | 200 | 20307 | 0 | 53% | 35% |
| this | 1 | 4 | true | 200 |  115490 | 0 | - | 90% |
| Nginx | 1 | 4 | true | 200 | 76106 | 0 | - | 93% |

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


## 2. Configureration Modular
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

For the test case: two threads write a file at the same time by using different locks.
The experiment measured the write speed and CPU usage of different types of mutexes used for thread synchronization. 
| Mutex Type | write speed | CPU usage |
|----------|----------|----------|
| NonMutex | 20 M/s | high |
| Mutex | 5.5 M/s | low |
| Spinlock | 7 M/s | high |
| CASlock | 8 M/s | high |

The experiment shows that while NonMutex provides the highest write speed at the cost of high CPU usage and thread safety, 
Mutex offers lower write speed with efficient CPU usage, whereas Spinlock and CASlock offer moderate write speeds with high CPU usage, suitable for short critical sections.

## 4. Fiber
Fibers are a type of lightweight thread that can be managed and scheduled by the application rather than the operating system. 
They provide a way to implement cooperative multitasking where the currently running fiber yields control explicitly, 
allowing other fibers to run. This can be useful in scenarios where fine-grained control over scheduling and execution order is needed without the overhead of full-fledged threads.
Adopt ucontext libary and Callback mechanism to encapsulate a flexible and robust fiber. 

Fiber is designed based on different states and a state machine, which ensures a reliable schduling proccess.
* **INIT**: This is the initial state of the fiber. When a fiber is created but not yet started, it is in the INIT state.
* **HOLD**: This state indicates that the fiber is currently not running and has been voluntarily suspended. It is not ready to run until explicitly resumed.
* **EXEC**: The fiber is currently running and executing its function. When a fiber is actively executing its code, it is in the EXEC state. This indicates that the fiber is the one currently being processed by the CPU.
* **TERM**: The fiber has completed its execution and has terminated. Once a fiber's function has finished executing, it enters the TERM state. This indicates that the fiber's lifecycle is complete, and it can be cleaned up or reset.
* **READY**: The fiber is ready to run but is not currently executing. It is waiting to be scheduled.
* **EXCEPT**: The fiber has encountered an exception during execution.

![fiber state machine](https://github.com/Disappear101/hight-performance-server/assets/105203326/55ca3b3f-b63c-4405-bf5a-6d2f83d55e43)

The lifecycle of a fiber is illustrated by the fiber state machine diagram. A fiber starts in the INIT (initialization) state and transitions to EXEC (executing) via the swap in action. During execution, it can yield to hold, moving to the HOLD (paused) state. The fiber is able to be resumed and transitions back to the EXEC state from the HOLD state. Upon completion, the fiber performs the terminate action to move to TERM (terminated). If an exception occurs during execution, it transitions to EXCEPT (exception) with the catch exception action and eventually swap out to exit(end of lifecycle). User can also yield to READY state from TERM or EXCEPT, so that the ready fiber can be scheduled, a fiber transitions back to EXEC from READY. These transitions and actions ensure organized scheduling, execution, and exception handling within fiber-based concurrency systems.




## 5. Fiber Scheduler
The fiber scheduler manages the execution of fibers across multiple threads, ensuring efficient scheduling, execution, and synchronization.
A M-thread N-fiber mode is used to inprove the performance.

Every thread at least has two basic fibers and task fibers：
* Scheduler fiber： Scheduler fiber is integral to managing the lifecycle and execution of fibers within a multithreaded environment.
  It ensures efficient task scheduling, execution, synchronization, and resource management. By coordinating fibers and threads,
  the Scheduler enables cooperative multitasking, providing a robust framework for concurrent execution. Switching between fibers must be done through the scheduler fiber, so
  task fibers are not allowed to be directly switched, which prevent from confusion over executive ownership.
* Idle fiber: The idle fiber in a fiber-based scheduler plays a critical role in ensuring efficient CPU utilization and proper task management when
  there are no immediate tasks to execute. The idle function in class scheduler is a virtual function so that it is able to overide it in a customized way to adapt different scenario.
* Task fiber: Task fiber is the carrier of the real tasks, as well as the objects to be scheduled.

![scheduler state machine](https://github.com/Disappear101/hight-performance-server/assets/105203326/b2917b73-fed5-4812-abd5-8d3471697e08)

The idea of fiber scheduler is similar with thread-pool architecture. The tasks are submitted in callback function or fiber form to task list where scheduler uses multi-thread to fetch the tasks.
In each thread, the scheduling work follows the above state machine.
![schedule drawio](https://github.com/user-attachments/assets/d4370105-31e6-4a9b-bf66-7da2cd0820cc)

## 6. Timer

Timers play a crucial role in I/O (Input/Output) operations for several reasons. They help manage and optimize the performance of systems that deal with I/O tasks, ensuring efficiency, reliability, and responsiveness.
The purposes of customizing timer are:
* **Efficient Resource Utilization**: By setting timeouts and periodically checking the status of I/O operations, systems can free up resources tied to stalled or slow operations, improving overall efficiency.
* **Non-Blocking I/O**: Timers are integral to implementing non-blocking I/O operations, allowing the system to continue processing other tasks while waiting for I/O operations to complete. This improves the responsiveness and throughput of the application.
* **Event-Driven Architecture**: In event-driven systems, timers are used to schedule and manage events, triggering I/O operations at the appropriate times and handling their completion asynchronously.
* **Task Scheduling**: Timers can be used to schedule I/O operations at specific times or intervals, ensuring that tasks are performed in a timely manner.

## 7. IOManager

IOManager inherits from Fiber scheduler and Time Manager, overiding idle, tickle and stopping method. The idle method has been implemented based on event loop(epoll_wait), with a specified timeout(maximun 3s). The tickle method is implemented based on a bidirectional pipe to generate a dummy event so that event loop can be woke up to schedule real task. The stopping method is to check whether the idle is stopping. In addition, IOManager implemented io-specified event management methods including addEvent, cancelEvent and delEvent. 
IOManager maintains a fdcontext vector used for add, delete, cancel and execute event corresponding fiber or callback function.

![iomanager drawio](https://github.com/user-attachments/assets/e12df75b-77bb-4132-93cb-73f151dfbca5)

The above figure showcase the io-specified event management flow(delete and cancel vice versa) and overided idle function. Fd, event and callback function is the inputs of add event function used for registering event. 
Management of event includes following steps:
* update events and callback function/fiber of maintained fdcontext indexed by fd.
* set epoll_event with updated events and updated fdcontext.
* add epoll event to be handled.

Idle state is epoll_wait used for waiting for events. Once events or epoll timeout is coming, idle function will proceed to execute expired timeout callback function, handle active events and then swap out to scheduler fiber.

## 8. Hook

The hooked io operations was implemented to manage io in a customized way that io operations run under non-block mode and their fibers can be swapped out/in when io operations are waiting status/triggered status.

![hook](https://github.com/Disappear101/hight-performance-server/assets/105203326/9d8e0eec-f7aa-4270-9e02-f61f395c654a)


## 9. ByteArray
![memory_pool drawio](https://github.com/user-attachments/assets/4a5876ce-a35c-41c4-b325-e733fd5d158e)


## 10.Http
* The class diagram of the whole server architcture including httpserver, tcpserver and websocketserver.
![http_class_diagram drawio](https://github.com/user-attachments/assets/9a214311-549d-46a3-bda6-e0d81e8f8e5a)




