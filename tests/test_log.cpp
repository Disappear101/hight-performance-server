#include <iostream>
#include "../src/log.h"
#include "../src/util.h"
#include <memory>

int main(int argc, char** argv) {
    tao::Logger::ptr logger = std::make_shared<tao::Logger>();
    logger->addAppender(tao::LogAppender::ptr(new tao::StdoutLogAppender));

    tao::FileLogAppender::ptr file_appender(new tao::FileLogAppender("./log.txt"));
    tao::LogFormatter::ptr fmt(new tao::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(tao::LogLevel::ERROR);

    logger->addAppender(file_appender);

    tao::LogEvent::ptr event(new tao::LogEvent(logger, tao::LogLevel::DEBUG, __FILE__, __LINE__, 0, tao::GetThreadId(), tao::GetFiberId(), time(0), "thread0"));
    event->getSS() << "hello tao log";
    logger->log(tao::LogLevel::DEBUG, event);
    std::cout << "hello tao log" << std::endl;

    //logger->log(tao::LogLevel::DEBUG, event);

    TAO_LOG_INFO(logger) << "test macro";
    TAO_LOG_ERROR(logger) << "test macro error";

    //tao::tao_fmt_log_print(logger, tao::LogLevel::ERROR, "test fmt printed function in %s level case %d", "error", 1);
    // TAO_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l = tao::LoggerMgr::GetInstance()->getLogger("xx");
    l->addAppender(tao::LogAppender::ptr(new tao::StdoutLogAppender));
    TAO_LOG_DEBUG(l) << "xxx";

    TAO_LOG_INFO(TAO_LOG_ROOT()) << "test TAO_LOG_ROOT!";
    return 0;
}
