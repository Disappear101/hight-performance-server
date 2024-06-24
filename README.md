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


## 2. Configure Modular

## 3. Thread and Mutex

## 4. Coroutines

## 5. Coroutines Scheduler

## 6. Timer

## 7. IOManager

## 8. Hook

## 9. ByteArray

## 10.Http


reference:
https://github.com/sylar-yin/sylar
