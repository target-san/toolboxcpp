#pragma once

#define $SourceLocation (::log_facade::util::SourceLocation(__FILE__, __LINE__, __FUNCTION__))

namespace log_facade
{
namespace util
{
/** Common-use structure which defines location in source code
 *  Defined in utils because used commonly both in logging and error handling (TBD)
 */
struct SourceLocation
{
    SourceLocation() = default;

    SourceLocation(const char* file, int line, const char* func)
        : file(file)
        , line(line)
        , func(func)
    { }

    const char* file = nullptr;
    int         line = 0;
    const char* func = nullptr;
};

}
}
