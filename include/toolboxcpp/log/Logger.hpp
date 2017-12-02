#pragma once

#include <chrono>
#include <memory>
#include "../Log.hpp"

namespace log_facade
{
namespace logger
{
    /** Contains basic metadata about log record, needed to determine if record should be written
     *  Passed to Logger's `is_enabled` method
     */
    struct Metadata
    {
        Severity    severity;
        Channel     channel;
        Location    location;
    };
    /// Timestamp type for logger record
    using Timestamp = std::chrono::system_clock::time_point;
    /** Contains initial metadata and additional info which is
     *  computed when log record is being written
     */
    struct Record: public Metadata
    {
        Timestamp   timestamp;
    };
    /** Polymorphic interface for all logger implementations
     */
    class Logger
    {
    public:
        /** Checks if logging of messages with provided metadata is enabled
         *  @param  metadata    Message metadata
         *  @return             `true` if message will be written, `false` otherwise
         */
        virtual bool is_enabled(Metadata const& metadata) = 0;
        /** Write log message to logger, using provided logging record metadata
         *  @param  record  Log record metadata
         *  @param  writer  Message writer func which accepts reference to STL stream and writes message into it
         */
        virtual void write(Record const& record, WriterFunc writer) = 0;
        /** Destructor
         */
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
    void set_logger_pointer(Logger* logger);
    /** Sets any object compatible with logger interface as current logger
     *
     *  Logger object doesn't need to derive from `Logger` interface, as it'll be
     *  packed into proper compatible type automatically
     *
     *  @param      logger              Logger object which will be set as current logger
     *  @exception  std::logic_error    If logger was already initialized
     */
    template<typename L>
    void set_logger(L&& logger)
    {
        struct Box: public Logger
        {
            Box(L&& logger)
                : logger(std::forward<L>(logger))
            { }
        
            bool is_enabled(Metadata const& meta) override
            {
                return logger.is_enabled(meta);
            }
            void write(Record const& rec, WriterFunc writer) override
            {
                logger.write(rec, writer);
            }
        
            typename std::decay<L>::type logger;
        };
        // Boxed logger is packed into unique_ptr
        std::unique_ptr<Box> box(new Box(std::forward<L>(logger)));
        // If exception would occur, boxed logger will be deleted
        set_logger_pointer(box.get());
        // If no exception would occur, box pointer will be released
        box.release();
    }
} // namespace logger
} // namespace log_facade
