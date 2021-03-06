#include <algorithm>
#include <atomic>
#include <stdexcept>

#include <toolboxcpp/log/Log.hpp>
#include <toolboxcpp/log/Logger.hpp>

namespace toolboxcpp
{
namespace log
{
/*
    Globally shared logger instance, with side-attached destructor
*/
namespace {
    std::atomic<Logger*> g_logger;

    void initMeta(Severity sev, Channel chan, Location loc, Metadata& meta)
    {
        meta.severity = std::min(std::max(sev, Severity::None), Severity::Trace);
        meta.channel  = chan ? chan : "";
        meta.location.file = loc.file ? loc.file : "<unknown>";
        meta.location.line = std::max(0, loc.line);
        meta.location.func = loc.func ? loc.func : "";    
    }

    void initRecord(Severity sev, Channel chan, Location loc, Record& rec)
    {
        initMeta(sev, chan, loc, rec);
        rec.timestamp = std::chrono::system_clock::now();
    }
}

    void set_logger_pointer(Logger* logger)
    {
        if(!logger)
            throw std::invalid_argument("logger");
        Logger* expected = nullptr;
        if(!g_logger.compare_exchange_weak(expected, logger, std::memory_order_acq_rel, std::memory_order_relaxed))
            throw std::logic_error("Logger already initialized");
    }

namespace impl
{
    bool is_enabled(Severity sev, Channel chan, Location loc)
    {
        Logger* logger = g_logger.load(std::memory_order_relaxed);
        if(logger == nullptr)
            return false;
        Metadata meta;
        initMeta(sev, chan, loc, meta);
        return logger != nullptr && logger->is_enabled(meta);
    }

    void write(Severity sev, Channel chan, Location loc, WriterFunc writer)
    {
        Logger* logger = g_logger.load(std::memory_order_relaxed);
        if(logger == nullptr)
            return;
        Record record;
        initRecord(sev, chan, loc, record);
        logger->write(record, writer);
    }
} // namespace impl

} // namespace log
} // namespace toolboxcpp
