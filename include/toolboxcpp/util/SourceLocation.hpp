#pragma once
/**
    Expands into structure which contains source file path, line number and function name
    of the position where macro was expanded. Shorthand for __FILE__, __LINE__, __FUNCTION__ triple.
 */
#define $SourceLocation (::toolboxcpp::util::SourceLocation(__FILE__, __LINE__, __FUNCTION__))

namespace toolboxcpp
{
namespace util
{
/**
    Common-use structure which defines location in source code
    Defined in utils because used commonly both in logging and error handling (TBD)
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
