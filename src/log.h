#ifndef _SYLAR_LOG_
#define _SYLAR_LOG_

#include <iostream>
#include <string>
#include <unordered_map>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>
#include "util.h"
#include "singleton.h"

//when exit if, LogEventWrap will deconstruct 
#define SYLAR_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level) \
        sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0), "thread0")).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)

#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)

#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)

#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)

#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {

class Logger;

class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* ToString(LogLevel::Level level);
};

class LogEvent{
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name);

    const char* getFile() const { return m_file;}

    int32_t getLine() const { return m_line;}
    uint32_t getElapse() const { return m_elapse;}
    uint32_t getThreadId() const { return m_threadId;}
    uint32_t getFiberId() const { return m_fiberId;}
    uint64_t getTime() const { return m_time;}
    std::string getContent() const { return m_ss.str();}
    std::stringstream& getSS() { return m_ss;}
    LogLevel::Level getLevel() const { return m_level;}
    std::string getThreadName() const { return m_threadName;}
    std::shared_ptr<Logger> getLogger() const { return m_logger;}

    template<typename... Args>
    void format(const char* fmt, Args&&... args) {
        formatting(fmt, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    void formatting(const char* fmt, Args&&... args) {
        int size = snprintf(nullptr, 0, fmt, args...) + 1;//+1 for '\0'
        if (size <= 0) {
            throw std::runtime_error("Error during formatting");
        }
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, fmt, args...);
        m_ss << std::string(buf.get(), buf.get() + size - 1);
    }
private:
    const char* m_file = nullptr;       //filename
    int32_t m_line = 0;                 //line number
    uint32_t m_elapse = 0;              //program luanch time(ms)
    uint32_t m_threadId = 0;            //threadid
    uint32_t m_fiberId = 0;             //coroutines ID
    uint64_t m_time = 0;                //time stamp
    std::string m_threadName;           //thread Name
    //Logger::ptr m_logger;               //logger
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;            //log level
    std::stringstream m_ss;             //log stream         
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream & getSS() {return m_event->getSS(); };
private:
    LogEvent::ptr m_event;
};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        FormatItem(const std::string& fmt = std::string()) {};
        virtual ~FormatItem(){};
        virtual void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init();
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};

class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {};

    virtual void log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val) {m_formatter = val;}

    LogFormatter::ptr getFormatter() {return m_formatter;};

    void setLevel(LogLevel::Level val) {m_level = val;}

    LogLevel::Level getLevel() const {return m_level; }
protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};


class Logger : public std::enable_shared_from_this<Logger>{
public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const std::string& name = std::string{"root"}, LogLevel::Level level = LogLevel::DEBUG);
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr e);
    void info(LogEvent::ptr e);
    void warn(LogEvent::ptr e);
    void error(LogEvent::ptr e);
    void fatal(LogEvent::ptr e);
    
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const {return m_level;};
    void setLevel(LogLevel::Level level) {m_level = level;};

    const std::string& getName() const {return m_name;};
private:
    std::string m_name;                         //log name
    LogLevel::Level m_level;                    //log level   
    std::list<LogAppender::ptr> m_appenders;    //appender list
    LogFormatter::ptr m_formatter;              //formatter
};

//appender of log output to console
class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    virtual void log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override;
};

//appender of log output to file
class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string& filename);
    virtual void log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override;

    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
    //last open time
    uint64_t m_lastTime = 0;
};

class LoggerManager {
public:
    LoggerManager();

    Logger::ptr getLogger(const std::string& name);

    void init();

    Logger::ptr getRoot() const { return m_root;}

private:
    Logger::ptr m_root;

    std::unordered_map<std::string, Logger::ptr> m_loggers;

};

template<typename... Args>
inline void sylar_fmt_log_print(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* fmt, Args... args) {
    if (logger->getLevel() <= level) {
        sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(logger, level, 
                        __FILE__, __LINE__, 0, GetThreadId(), GetFiberId(), 
                        time(0), "thread0")).getEvent()->format(fmt, std::forward<Args>(args)...);
    }
}

using LoggerMgr = sylar::Singleton<LoggerManager>;

};

#endif