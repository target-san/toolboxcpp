#include <diag/Log.hpp>
#include <diag/Logger.hpp>

#include <atomic>
#include <stdexcept>
#include <cstdarg>

namespace common
{
namespace diag
{
/*
    Globally shared logger instance, with side-attached destructor
*/
namespace {

std::atomic<Logger*> g_logger(nullptr);

class LoggerDestructor
{
public:
    LoggerDestructor() = default;
    ~LoggerDestructor()
    {
        // extract pointer from g_logger and drop it, replacing with nullptr
        // TODO: select proper ordering
        delete g_logger.exchange(nullptr);
    }
};

LoggerDestructor g_loggerDestructor;

}

void set_logger(std::unique_ptr<Logger> logger)
{
    if(!logger)
        throw std::invalid_argument("logger");
    Logger* expected = nullptr;
    // TODO: select proper ordering
    if(g_logger.compare_exchange_weak(expected, logger.get()))
    {
        logger.release();
    }
    else
    {
        throw std::logic_error("Logger already initialized");
    }
}

namespace impl
{

bool is_enabled(Severity severity, Target target)
{
    // TODO: select proper ordering
    Logger* logger = g_logger; // obtain local pointer
    return logger != nullptr && logger->is_enabled(Metadata{ severity, target });
}

void write(Severity severity,Target target, Location loc, const char* format, ...)
{
    // TODO: select proper ordering
    Logger* logger = g_logger;
    if(logger == nullptr)
    {
        return;
    }
    va_list args;
    va_start(args, format);
    // TODO: format message from formatter args
    std::string message;
    va_end(args);
    Record record{
        Metadata { severity, target },
        loc,
        std::chrono::system_clock::now(),
        message
    };
    logger->write(record);
}

} // namespace impl

} // namespace diag
} // namespace common
