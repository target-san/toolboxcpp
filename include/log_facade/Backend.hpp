#pragma once

#include <chrono>
#include <memory>
#include "Log.hpp"

namespace log_facade
{

struct Metadata
{
    Severity    severity;
    Channel     channel;
    Location    location;
};

using Timestamp = std::chrono::system_clock::time_point;

struct Record: public Metadata
{
    Timestamp   timestamp;
};

class Logger
{
public:
    virtual bool is_enabled(Metadata const&) = 0;
    virtual void write(Record const&, WriterFunc writer) = 0;

    virtual ~Logger() {}
};

void set_logger(std::unique_ptr<Logger>);

} // namespace diag
