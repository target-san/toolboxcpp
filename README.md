# log_facade

A small C++11 library for covering your code with log messages.
Unlike existing huge libraries like `pantheios`, `Boost.Log`, `log4cpp` etc.,
it focuses only on logging macros in user's code and provides single
initialization point, with easily understandable interface, to make them alive.
Actually, library user is intended to use this library for traces and then
implement full logger backend based on one of aforementioned libraries.

There are some simple "combinators", but they're placed in library mostly for conveinence,
in case someone wants very simple logging out-of-the-box.

Heavily inspired by Rust's `log` package (https://crates.io/crates/log).

## Note on macros naming

Macros which start with `$` character look a bit unusual.
Although, this way they won't clash with normal identifiers and are easily distinguishable in code.
So for me it's purely aesthetic choice.

## How to use

```cpp
#include <log_facade/Log.hpp>

void do_stuff(int first, double second)
{
    $log_trace("Function started");
    $log_debug("first: ", first, " second: ", second);

    // ... do actual stuff

    $log_trace("Function ended");
}
```

## How to implement logger

TODO

# Reference

Forward note: all logging macros are designed in such a way that all objects
subject to logging are evaluated only if logger reports that message will be sent somewhere.
Also, debug and trace macros are completely elided in release builds.

## Basic macros

The most used set of macros. Actually, you most probably need only these.

- `$log_error(...)`
- `$log_warn(...)`
- `$log_info(...)`
- `$log_debug(...)`
- `$log_trace(...)`

Each of these sends message to logger, with relevant severity level.
Each accepts variadic arguments, which will be written to log.

Please note that actual way objects are formatted depends on selected formatter.
For convenience, default formatter behavior is stream output operator.
Custom formatter stuff is discussed later.

## Basic channels support

Besides severity level and message location, `log_facade` has such concept as 'channel'
Channel is simply a user-defined string which somehow describes current scope of messages,
or their purpose, or whatever user deems applicable.

Channel can be established for current scope via `$LogChannel($chan)` macro, where `$chan` should be
string literal. This macro can be used in global scope or as part of class declaration -
but not inside function body.   

## Messaging macros with explicit location

- `$log_error_at($channel, $location, ...)`
- `$log_warn_at($channel, $location, ...)`
- `$log_info_at($channel, $location, ...)`
- `$log_debug_at($channel, $location, ...)`
- `$log_trace_at($channel, $location, ...)`

Each of these macros accepts logging channel and location explicitly, and then just treats all ellipsis arguments as message.

## Fundamental macro

`$log_perform_write($severity, $channel, $location, ...)`

This macro does not deduce any parameters from context and accepts them explicitly.
All other logging macros are in fact implemented through this one

# LICENSE

This project is licensed under [LICENSE](MIT license).
Please note that Google Test package, which is used to build and run unit tests,
is neither a part of core library, nor its required component. It's licensed separately.
