# log_facade

A small C++11 library which allows to place logging calls in your code.
Heavily inspired by Rust's `log` package (https://crates.io/crates/log).
Unlike existing huge libraries like `pantheios`, `Boost.Log`, `log4cpp` etc.,
it provides only primitive loggers by default and, basically, not intended
to provide full-fledged backend.

## Frontend

Frontend part is described by `include/log_facade/Log.hpp` header. Contents:

- Log message metadata. Properties which describe logging message in-place, statically
  - Severity: `log_facade::Severity` enum, declares 5 main severity levels (error, warning, information, debug, trace)
  - Channel:  `log_facade::Channel`, string literal which is usually set within context and resolved by logging statements implicitly.
    It's entirely up to client's code what to put there. Initialized in scope with `$LogChannel` macro and queried with `$LogCurrentChannel` macro.
  - Location: `log_facade::Location`, structure which contains file name, line number and function name. Usually constructed
    with `$LogCurrentLocation` macro and filled with compiler macros `__FILE__`, `__LINE__`, `__FUNC__`, respectively.
- Logging macros
  - `$log_error`, `$log_warn`, `$log_info`, `$log_debug`, `$log_trace` simply write all arguments as single message.
  - `$log_error_at`, `$log_warn_at`, `$log_info_at`, `$log_debug_at`, `$log_trace_at` behave similar, but accept channel and location
    as explicit arguments. This is sometimes needed when logging is performed by some wrapper code or macro, so that message can
    receive location metadata where that wrapper was called, and not where actual logging statement resides.
  - `$LogChannel(...)` - declares logging channel till the end of current scope, such that it's picked by logging macros.
  - `$LogCurrentChannel`, `$LogCurrentLocation` - expands into current channel and current location expressions.
  
