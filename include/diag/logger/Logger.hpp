#pragma once

#include <chrono>
#include <memory>
#include "../Log.h"

namespace diag
{

using Timestamp = std::chrono::system_clock::time_point;

struct Metadata
{
    Severity severity;
    Target   target;
};

struct Record
{
    Metadata    metadata;
    Location    location;
    Timestamp   timestamp;
    StrSpan     message;
};

class Logger
{
public:
    virtual bool isEnabled(Metadata const&) = 0;
    virtual void write(Record const&) = 0;

    virtual ~Logger() {}
};

void setLogger(std::unique_ptr<Logger>);

} // namespace diag
