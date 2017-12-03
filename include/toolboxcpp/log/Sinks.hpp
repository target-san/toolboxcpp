#pragma once
/** Standard logging sinks
 */
#include <toolboxcpp/log/Logger.hpp>

#include <iostream>
#include <fstream>

namespace toolboxcpp
{
namespace log
{
    /** Writes all messages to standard output stream
     */
    struct StdOutLogger
    {
        StdOutLogger() {}

        bool is_enabled(Metadata const&) { return true; }

        void write(Record const&, WriterFunc writer)
        {
            writer(std::cout);
            std::cout << std::endl;
        }
    };
    /** Writes all messages to standard error stream
     */
    struct StdErrLogger
    {
        StdErrLogger() {}

        bool is_enabled(Metadata const&) { return true; }

        void write(Record const&, WriterFunc writer)
        {
            writer(std::cerr);
            std::cerr << std::endl;
        }
    };
    /** Writes all messages to file stream
     */
    struct FileLogger
    {
    public:
        FileLogger(const char* path, bool append)
            : _file(path, append ? std::ios_base::app : std::ios_base::out)
        { }

        bool is_enabled(Metadata const&) { return true; }

        void write(Record const&, WriterFunc writer)
        {
            writer(_file);
            _file << std::endl;
        }
    private:
        std::ofstream _file;
    };
}
}
