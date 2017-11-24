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
 *  
 *  Logger is passed as bare pointer, which is never deleted.
 *  As a result, logger can be supplied inside both as on-heap object
 *  and as a pointer to static object.
 *  In former case, on-heap object will exist for the whole lifetime of program.
 *  In latter case, pointed-to object is never deleted.
 *  Such approach should allow to cover more scenarios.
 *  
 *  @param      logger                  On-heap logger object
 *  @exception  std::invalid_argument   If logger is nullptr
 *  @exception  std::logic_error        If logger was already initialized
 */
void set_logger(Logger* logger);

} // namespace diag
