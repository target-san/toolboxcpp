#pragma once
/** Set of useful combinators and wrappers for constructing your own logger implementation
 *  Completely independent of any kind of concrete implementation
 */
#include <string>
#include <tuple>
#include <utility>

#include "Log.hpp"
#include "Backend.hpp"
#include "../util/FoldTuple.hpp"

namespace log_facade
{
/** Wraps any type which conforms to Logger concept
 *  into type which implements Logger interface
 *  In most cases, dynamic invocation is used only once,
 *  when public functions delegate to concrete logger implementation,
 *  and not needed inside composite logger itself - as all types are usually
 *  known there
 *  @tparam L Type of actual logger stored inside        
 */
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
    virtual void write(Record const& rec, WriterFunc writer) override
    {
        _logger.write(rec, writer);
    }

private:
    L _logger;
};
/** Packs any logger-compatible type into LoggerBox
 *  and sets as current logger
 *  @tparam L       Logger type being wrapped
 *  @param  logger  Logger being wrapped
 */
template<typename L>
void set_logger(L&& logger)
{
    using Nested = typename std::decay<L>::type;
    set_logger(new LoggerBox<Nested>(std::forward<L>(logger)));
}

template<typename... Logs>
class MultiLogger
{
private:
    struct IsEnabled
    {
        Metadata const& meta;

        template<typename T>
        bool operator()(bool enabled, T&& logger)
        {
            return enabled || logger.is_enabled(meta);
        }
    };

    struct Write
    {
        Record const& record;
        WriterFunc writer;

        template<typename T>
        int operator()(int, T&& logger)
        {
            if(logger.is_enabled(record))
                logger.write(record, writer);
            return 0;
        }
    };
public:

    template<typename... Args>
    MultiLogger(Args&&... args)
        : _loggers(std::forward<Args>(args)...)
    { }

    bool is_enabled(Metadata const& meta)
    {
        return util::fold_tuple(_loggers, false, IsEnabled { meta });
    }

    void write(Record const& rec, WriterFunc writer)
    {
        util::foldTuple(_loggers, 0, Write{ rec, writer });
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

    void write(Record const& record, WriterFunc writer)
    {
        _logger.write(record, writer);
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

    void write(Record const& rec, WriterFunc writer)
    {
        auto format_proxy = [&] (std::ostream& ost) { _formatter(ost, rec, writer); };
        _logger.write(rec, writer);
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
