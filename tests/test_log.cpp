#include <iostream>
#include "../src/log.h"
#include "../src/util.h"
#include <memory>

int main(int argc, char** argv) {
    sylar::Logger::ptr logger = std::make_shared<sylar::Logger>();
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);

    logger->addAppender(file_appender);

    sylar::LogEvent::ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG, __FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0), "thread0"));
    event->getSS() << "hello sylar log";
    logger->log(sylar::LogLevel::DEBUG, event);
    std::cout << "hello sylar log" << std::endl;

    //logger->log(sylar::LogLevel::DEBUG, event);

    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";

    sylar::sylar_fmt_log_print(logger, sylar::LogLevel::ERROR, "test fmt printed function in %s level case %d", "error", 1);
    // SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    l->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    SYLAR_LOG_DEBUG(l) << "xxx";

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "test SYLAR_LOG_ROOT!";
    return 0;
}
