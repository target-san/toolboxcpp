#pragma once

#include <chrono>
#include <memory>
#include "Facade.hpp"

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
    Timestamp                           timestamp;
    util::FuncRef<void(std::ostream&)>  message;
};

class Logger
{
public:
    virtual bool is_enabled(Metadata const&) = 0;
    virtual void write(Record const&) = 0;

    virtual ~Logger() {}
};

void set_logger(std::unique_ptr<Logger>);

} // namespace diag
