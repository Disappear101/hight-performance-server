#include "log.h"

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

Logger::Logger(const std::string& name = "root")
    :m_name(name){

}

void Logger::addAppender(LogAppender::ptr appender){
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
    if (level >= m_level) {
        for (auto & i : m_appenders) {
            i->log(self, level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr e){
    debug(e);
}
void Logger::info(LogEvent::ptr e){
    info(e);
}
void Logger::warn(LogEvent::ptr e){

}
void Logger::error(LogEvent::ptr e){

}
void Logger::fatal(LogEvent::ptr e){

}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){

}

void FileLogAppender::log(std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
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
    size_t last_pos = 0;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%'){
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if (i + 1 < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
            }
        }
        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while ( n < m_pattern.size())
        {
            if (isspace(m_pattern[n])){
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '('){
                    str = m_pattern.substr(i + 1, n - i);
                    fmt_status = 1;//parse format
                    fmt_begin = n + 1;
                    ++n;
                    continue;
                }
            }
            if (fmt_status == 1){
                if (m_pattern[n] == ')') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin);
                    fmt_status = 2;
                    ++n;
                    break;
                }
            }
        }
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
            }
            str = m_pattern.substr(i + 1, n - 1 - i);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } else if (fmt_status == 2){
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }
    }
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    //%m -- message
    //%p -- level
    //%r -- running time
    //%c -- log name
    //%t -- thread
    //%n -- enter
    //%d -- time
    //%f -- filename
    //%l -- line number
}
class MessageFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class LoggerFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%S")
        :m_format(format){};
    void format(std::ostream& os, std::shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getTime();
    }
private:
    std::string m_format;
};

}
