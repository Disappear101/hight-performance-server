#ifndef _SYLAR_LOG_
#define _SYLAR_LOG_

#include <iostream>
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <vector>

namespace sylar {

class Logger;

class LogEvent{
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent();

    const char* getFile() const { return m_file;}

    int32_t getLine() const { return m_line;}
    uint32_t getElapse() const { return m_elapse;}
    uint32_t getThreadId() const { return m_threadId;}
    uint32_t getFiberId() const { return m_fiberId;}
    uint64_t getTime() const { return m_time;}
    std::string getContent() const { return m_content;}
private:
    const char* m_file = nullptr;       //filename
    int32_t m_line = 0;                 //line number
    uint32_t m_elapse = 0;              //program luanch time(ms)
    uint32_t m_threadId = 0;            //threadid
    uint32_t m_fiberId = 0;             //coroutines ID
    uint64_t m_time = 0;                //time stamp
    std::string m_content;            
};

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

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem(){};
        virtual void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init();
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {};

    virtual void log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val) {m_formatter = val;};
protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};


class Logger {
public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const std::string& name = "root");
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
    std::string m_name;                     //log name
    LogLevel::Level m_level;       
    std::list<LogAppender::ptr> m_appenders; //appender list
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