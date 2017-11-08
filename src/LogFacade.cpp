#include "../include/log_facade/Log.hpp"
#include "../include/log_facade/Backend.hpp"

#include <algorithm>
#include <atomic>
#include <stdexcept>

namespace log_facade
{
/*
    Globally shared logger instance, with side-attached destructor
*/
namespace {

std::atomic<Logger*> g_logger(nullptr);

void initMeta(Severity sev, Channel chan, Location loc, Metadata& meta)
{
    meta.severity = std::min(std::max(sev, Severity::None), Severity::Trace);
    meta.channel  = chan ? chan : "";
    meta.location.file = loc.file ? loc.file : "<unknown>";
    meta.location.line = std::min(0, loc.line);
    meta.location.func = loc.func ? loc.func : "";    
}

void initRecord(Severity sev, Channel chan, Location loc, Record& rec)
{
    initMeta(sev, chan, loc, rec);
    rec.timestamp = std::chrono::system_clock::now();
}    

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

bool is_enabled(Severity sev, Channel chan, Location loc)
{
    // TODO: select proper ordering
    Logger* logger = g_logger; // obtain local pointer
    if(logger == nullptr)
        return false;
    Metadata meta;
    initMeta(sev, chan, loc, meta);
    return logger != nullptr && logger->is_enabled(meta);
}

void write(Severity sev, Channel chan, Location loc, WriterFunc writer)
{
    // TODO: select proper ordering
    Logger* logger = g_logger;
    if(logger == nullptr)
        return;

    Record record;
    initRecord(sev, chan, loc, record);
    logger->write(record, writer);
}

}

}
