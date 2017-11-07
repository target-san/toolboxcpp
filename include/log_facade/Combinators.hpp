#pragma once

#include <string>
#include <tuple>
#include <utility>

#include "Facade.h"
#include "Backend.h"
#include "../util/FoldTuple.h"

namespace log_facade
{

template<typename L>
class LoggerBox: public Logger
{
public:
    template<typename T>
    LoggerBox(T&& logger)
        : _logger(std::forward<T>(logger))
    { }    

    virtual bool is_enabled(Metadata const& meta) override
    {
        return _logger.is_enabled(meta);
    }
    virtual void write(Record const& rec) override
    {
        _logger.write(rec);
    }

private:
    L _logger;
};

template<typename L>
void set_logger(L&& logger)
{
    using Nested = typename std::decay<L>::type;
    set_logger(std::make_unique<LoggerBox<Nested>>(std::forward<L>(logger)));
}

template<typename... Logs>
class MultiLogger
{
public:

    template<typename... Args>
    MultiLogger(Args&&... args)
        : _loggers(std::forward<Args>(args)...)
    { }

    bool is_enabled(Metadata const& meta)
    {
        return util::foldTuple(_loggers, false, [&](auto acc, auto& logger) { return acc || logger.is_enabled(meta); });
    }

    void write(Record const& rec)
    {
        util::foldTuple(_loggers, 0, [&record](auto acc, auto& logger) {
            return (logger.is_enabled(rec) ? logger.write(rec) : (void()) ), acc;
        });
    }

private:
    std::tuple<Logs...> _loggers;
};

template<class... Args>
MultiLogger<std::decay_t<Args>...> make_multi_logger(Args&&... args)
{
    return MultiLogger<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

template<class Logger>
void set_loggers(Logger&& logger)
{
    set_logger(std::forward<Logger>(logger));
}

template<class Log0, class Log1, class... Logs>
void set_loggers(Log0&& log0, Log1&& log1, Logs&&... logs)
{
    set_loggers(
        make_multi_logger(std::forward<Log0>(log0), std::forward<Log1>(log1), std::forward<Logs>(logs)...)
    );
};

template<typename L, typename Fn>
class FilteredLogger
{
public:
    FilteredLogger(L logger, Fn filter)
        : _logger(std::move(logger))
        , _filter(std::move(filter))
    { }

    bool is_enabled(Metadata const& meta)
    {
        return _filter(meta) && _logger.is_enabled(meta);
    }

    void write(Record const& record)
    {
        _logger.write(record);
    }

private:
    L   _logger;
    Fn  _filter;
};

template<typename L, typename Fn,>
FilteredLogger<typename std::decay<L>::type, typename std::decay<Fn>::type>
make_filtered_logger(L&& logger, Fn&& filter)
{
    return FilteredLogger<typename std::decay<L>::type, typename std::decay<Fn>::type>
        (std::forward<L>(logger), std::forward<Fn>(filter));
}

template<typename L, typename Fn>
class FormattedLogger
{
public:
    FormattedLogger(L logger, Fn formatter)
        : _logger(std::move(logger))
        , _formatter(std::move(formatter))
    { }

    bool is_enabled(Metadata const& meta) { return _logger.is_enabled(meta); }

    void write(Record const& rec)
    {
        auto format_proxy = [&] (std::ostream& ost) { _formatter(ost, rec); };
        Record inner = rec;
        inner = format_proxy;
        _logger.write(inner);
    }

private:
    L   _logger;
    Fn  _formatter;
};

template<typename L, typename Fn>
FormattedLogger<typename std::decay<L>::type, typename std::decay<Fn>::type>
make_formatted_logger(L&& logger, Fn&& formatter)
{
    return FormattedLogger<typename std::decay<L>::type, typename std::decay<Fn>::type>
        (std::forward<L>(logger), std::forward<Fn>(formatter));
}

} // namespace diag
