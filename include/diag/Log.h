#pragma once
#include <ostream>
#include "../util/FuncRef.h"
/*
    Logging macros - use these to write messages to log
*/

// Hard, unrecoverable error
#define $logError(...) $logErrorAt($LogCurrentTarget, $LogCurrentLocation, __VA_ARGS__)
// An error which can be possibly handled somewhere up the code hierarchy
#define $logWarn( ...) $logWarnAt( $LogCurrentTarget, $LogCurrentLocation, __VA_ARGS__)
// Informational message
#define $logInfo( ...) $logInfoAt( $LogCurrentTarget, $LogCurrentLocation, __VA_ARGS__)
// Debug data, like state of some structure after operation
#define $logDebug(...) $logDebugAt($LogCurrentTarget, $LogCurrentLocation, __VA_ARGS__)
// Highly-detailed tracing message - function enter/leave, exception being wrapped with additional context etc.
#define $logTrace(...) $logTraceAt($LogCurrentTarget, $LogCurrentLocation, __VA_ARGS__)

/*
    Conditional logging macros - use these to write messages to log under certain condition
*/

#define $logErrorIf($cond, ...) (($cond) ? $logError(__VA_ARGS__) : (void()) )
// An error which can be possibly handled somewhere up the code hierarchy
#define $logWarnIf($cond,  ...) (($cond) ? $logWarn( __VA_ARGS__) : (void()) )
// Informational message
#define $logInfoIf($cond,  ...) (($cond) ? $logInfo( __VA_ARGS__) : (void()) )
// Debug data, like state of some structure after operation
#define $logDebugIf($cond, ...) (($cond) ? $logDebug(__VA_ARGS__) : (void()) )
// Highly-detailed tracing message - function enter/leave, exception being wrapped with additional context etc.
#define $logTraceIf($cond, ...) (($cond) ? $logTrace(__VA_ARGS__) : (void()) )

/** Establish named log target till the end of current translation unit
    Only string literals can be used
    Such 'target' may be used to filter logging messages
    
    @param  $identifier String literal which denotes current logging target
*/
#define $LogTarget($identifier) static constexpr ::$LogNS::Target __getLogTarget__(::$LogNS::impl::AdlTag) { return ($identifier); }
/**
    Logging macros which are specialized by severity but allow to specify custom target and location
    May be used in cases where logging macro is invoked through some intermediate code,
    and what matters is the invocation site of that code
    
    @param[in] $target      Target name
    @param[in] $location    File and line where logging happens
*/
#define $logErrorAt($target, $location, ...) $logWriteRaw(::$LogNS::Severity::Error,   $target, $location, __VA_ARGS__)
#define $logWarnAt( $target, $location, ...) $logWriteRaw(::$LogNS::Severity::Warning, $target, $location, __VA_ARGS__)
#define $logInfoAt( $target, $location, ...) $logWriteRaw(::$LogNS::Severity::Info,    $target, $location, __VA_ARGS__)
/*
    Two lowest levels of logging are compiled-in only in debug mode or if explicitly enabled via macro
*/
#if (defined _DEBUG) || (defined LOG_DETAILED)
#   define $logDebugAt($target, $location, ...) $logWriteRaw(::$LogNS::Severity::Debug, $target, $location, __VA_ARGS__)
#   define $logTraceAt($target, $location, ...) $logWriteRaw(::$LogNS::Severity::Trace, $target, $location, __VA_ARGS__)
#else
#   define $logDebugAt($target, $location, ...) (void())
#   define $logTraceAt($target, $location, ...) (void())
#endif
/** The most explicit log writing macro. Does not infer any info from its current context
    
    @param[in] $severity    log severity level
    @param[in] $target      log target location, like the name of current module
    @param[in] $location    file and line which should be used in log message as location
    @param[in] $first       First object to write, usually string; just ensures there's anything to write
*/
#define $logWriteRaw($severity, $target, $location, $first, ...) ( \
    ::$LogNS::impl::isEnabled($severity, $target) \
        ? ::$LogNS::impl::write($severity, $target, $location, $logFormat($first, __VA_ARGS__)) \
        : (void()) )

/** Substitutes with current 'target' defined in current scope
*/
#define $LogCurrentTarget (__getLogTarget__(::$LogNS::impl::AdlTag {}))
/** Substitutes with current loation object, which contains current file and line
*/
#define $LogCurrentLocation (::$LogNS::Location { __FILE__, __LINE__ })
/** Defines default log formatting method, which simply dumps all specified expressions to provided stream
    Any override must be a macro which accepts variadic number of arguments and generates callable
    which accepts `std::ostream&` and returns nothing
*/
#ifndef $logFormat
//  Contains implementation of defaultFormat which is a bit complicated to be shown here
#   include "Log.ipp"
/** @brief Default log formatting method
*/
#   define $logFormat(...) (::$LogNS::impl::defaultFormat(__VA_ARGS__))
#endif

/// Namespace where logging stuff is stored. In case one would want to relocate all this
#define $LogNS diag

namespace diag
{
// Importance level of log message
enum class Severity
{
    // In fact specifies no need for logging
    None,
    // Normal logging levels
    Error,
    Warning,
    Info,
    Debug,
    Trace,
    // Upper limit for the value of logging level
    _Count
};
/** @brief Defines log location
*/
struct Location
{
    const char* file;
    int         line;
};

using Target = const char*;

namespace impl
{
/**
    Checks if logging is enabled for provided severity level and target identifier

    @param  severity    Logging level
    @param  target      A string which identifies log invocation context; meaning is implementation-defined
    @return             true if message should be writter, false otherwise
*/
bool isEnabled(Severity severity, Target target);
/** @brief Deliver message to logging subsystem
  
    Writes specified message with specified metadata to log
    Not guaranteed to check if log is enabled for specified severity and target
    
    @param  severity    Logging level
    @param  target      A string which identifies log invocation context; meaning is implementation-defined
    @param  loc         File name and line number where logging happens
    @param  writer      Function which receives stream and writes logging message into it
*/
void write(Severity severity, Target target, Location loc, util::FuncRef<void(std::ostream&)> writer);
/// Enables ADL-based deduction on which "log target" function to use
struct AdlTag {};
// Returns default log target, nullptr in our case
static inline Target __getLogTarget__(...)
{
    return "";
}

} // namespace impl

} // namespace diag
