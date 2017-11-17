#pragma once

#define $SourceLocation (::util::SourceLocation { __FILE__, __LINE__, __FUNCTION__})

namespace util
{
/** Common-use structure which defines location in source code
 *  Defined in utils because used commonly both in logging and error handling (TBD)
 */
struct SourceLocation
{
    const char* file;
    int         line;
    const char* func;
};

}
