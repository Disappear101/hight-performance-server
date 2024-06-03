#include "log.h"
#include <map>
#include <functional>

namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch(level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break; 
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX 
    default: 
        return "UNKNOW"; 
    }
    return "UNKNOW";
}

LogEventWrap::LogEventWrap::LogEventWrap(LogEvent::ptr event) 
    :m_event(event) {
}

//deconstructing stage execute log output 
LogEventWrap::LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class LoggerFormatItem : public LogFormatter::FormatItem {
public:
    LoggerFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format){
            if (m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        };
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        //os << event->getTime();
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str = std::string())
        : m_str(str) {}
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_str;
    }
private:
    std::string m_str;
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = std::string()) {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_threadName(thread_name)
    ,m_logger(logger)
    ,m_level(level) {
}


Logger::Logger(const std::string& name, LogLevel::Level level)
        :m_name(name)
        ,m_level(level){
    m_formatter.reset(new LogFormatter("%d%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    //m_appenders.push_back(std::make_shared<StdoutLogAppender>());
}

void Logger::addAppender(LogAppender::ptr appender){
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender){
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it){
        if (*it == appender) {
            m_appenders.erase(it);
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    auto self = shared_from_this();
    if (m_appenders.empty()) {
        std::cout << "No available appender! please add appender!" <<  std::endl;
        return;
    }
    if (level >= m_level) {
        for (auto & i : m_appenders) {
            i->log(self, level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr e){
    log(LogLevel::DEBUG, e);
}
void Logger::info(LogEvent::ptr e){
    log(LogLevel::INFO, e);
}
void Logger::warn(LogEvent::ptr e){
    log(LogLevel::WARN, e);
}
void Logger::error(LogEvent::ptr e){
    log(LogLevel::ERROR, e);
}
void Logger::fatal(LogEvent::ptr e){
    log(LogLevel::FATAL, e);
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        uint64_t now = event->getTime();
        if (now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }

        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen(){
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);

    return !!m_filestream;
}

void StdoutLogAppender::log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event) << std::endl;
    }
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern){
    init();
}
std::string LogFormatter::format(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event){
    std::stringstream ss;
    for (auto & i : m_items){
        i->format(ss, logger, level, event);
    }
    return ss.str();
}
//%xxx %xxx(xxx) %%
void LogFormatter::init(){
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int> > vec;
    //size_t last_pos = 0;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%'){
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if (i + 1 < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }
        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while ( n < m_pattern.size())
        {
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{'){
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;//parse format
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1){
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            //str = m_pattern.substr(i + 1, n - 1 - i);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } 
    }
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string & str)>> s_format_items = {
    #define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}
        //%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
        XX(m, MessageFormatItem),           //m: message
        XX(p, LevelFormatItem),             //p: log level
        XX(r, ElapseFormatItem),            //r: counter(ms)
        XX(c, LoggerFormatItem),            //c: loger name
        XX(t, ThreadIdFormatItem),          //t:thread id
        XX(n, NewLineFormatItem),           //n:new line
        XX(d, DateTimeFormatItem),          //d:date time
        XX(f, FilenameFormatItem),          //f:file name
        XX(l, LineFormatItem),              //l:line number
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:coroutine id
        XX(N, ThreadNameFormatItem),        //N:thread name
    #undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    //%m -- message
    //%p -- level
    //%r -- running time
    //%c -- log name
    //%t -- thread
    //%n -- enter(new line)
    //%d -- time
    //%f -- filename
    //%l -- line number
}

LoggerManager::LoggerManager() {
    m_root = std::make_shared<Logger>();
    m_root->addAppender(std::make_shared<StdoutLogAppender>());
    m_loggers[m_root->getName()] = m_root;

    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }

    Logger::ptr logger = std::make_shared<Logger>(name);
    //logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

void LoggerManager::init() {

}

}
