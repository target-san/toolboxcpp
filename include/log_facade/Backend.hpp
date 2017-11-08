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
/** @brief Set passed in object as current logger
 *  @param      logger                  On-heap logger object
 *  @exception  std::invalid_argument   If passed in unique_ptr is nullptr
 *  @exception  std::logic_error        If logger was already initialized
 */
void set_logger(std::unique_ptr<Logger> logger);

} // namespace diag
