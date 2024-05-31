#ifndef _SYLAR_LOG_
#define _SYLAR_LOG_

#include <iostream>
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <vector>

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

    void setFormatter(LogFormatter::ptr val) {m_formatter = val;};

    LogFormatter::ptr getFormatter() {return m_formatter;};

    void setLevel(LogLevel::Level val) {m_level = val;}
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
};

};

#endif