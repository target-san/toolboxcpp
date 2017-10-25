#pragma once

#include <string>
#include <tuple>
#include <utility>

#include "Log.hpp"
#include "Logger.hpp"
#include "Util.hpp"

namespace common
{
namespace diag
{

template<class... Logs>
class MultiLogger : public Logger
{
public:

    MultiLogger(Logs... loggers)
        : m_loggers(std::move(loggers)...)
    { }

    MultiLogger(MultiLogger const&) = delete;
    MultiLogger& operator=(MultiLogger const&) = delete;

    MultiLogger(MultiLogger &&) = default;
    MultiLogger& operator=(MultiLogger &&) = default;

    bool is_enabled(const Metadata& meta) override
    {
        return Fold(
            false,
            [&meta](auto acc, auto& logger) { return acc || logger.is_enabled(meta); }
        );
    }

    void write(const Record& record) override
    {
        Fold(
            0,
            [&record](auto acc, auto& logger) {
                return (logger.is_enabled(record.metadata) ? logger.write(record) : (void())), acc;
            }
        );
    }

private:
    std::tuple<Logs...> m_loggers;

    template<class T, class Fn, size_t I, size_t... Is>
    T fold_impl(T acc, Fn&& func, std::index_sequence<I, Is...>)
    {
        auto tmp = func(std::move(acc), std::get<I>(m_loggers));
        return fold_impl(std::move(tmp), std::forward<Fn>(func), std::index_sequence<Is...>());
    }

    template<class T, class Fn>
    T fold_impl(T acc, Fn&&, std::index_sequence<>)
    {
        return acc;
    }

    template<class T, class Fn>
    T fold(T acc, Fn&& func)
    {
        return FoldImpl(acc, std::forward<Fn>(func), std::make_index_sequence<sizeof...(Logs)>());
    }
};

namespace multi_logger
{

template<class... Args>
MultiLogger<std::decay_t<Args>...> make(Args&&... args)
{
    return MultiLogger<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

}

template<class Logger>
void set_loggers(Logger&& logger)
{
    set_logger(std::make_unique<std::decay_t<Logger>>(std::forward<Logger>(logger)));
}

template<class Log0, class Log1, class... Logs>
void set_loggers(Log0&& log0, Log1&& log1, Logs&&... logs)
{
    set_loggers(
        multi_logger::make(std::forward<Log0>(log0), std::forward<Log1>(log1), std::forward<Logs>(logs)...)
    );
};

template<class L>
class LoggerSeverityFilter : public Logger
{
public:
    LoggerSeverityFilter(Severity severity, L logger)
        : m_logger(std::move(logger))
        , m_severity(severity)
    { }


    bool is_enabled(const Metadata& meta) override { return meta.severity <= m_severity && m_logger.is_enabled(meta); }

    void write(const Record& record) override { m_logger.write(record); }

private:
    L m_logger;
    Severity m_severity;
};

namespace logger_severity_filter
{

template<class T>
LoggerSeverityFilter<std::decay_t<T>> make(Severity sev, T&& logger)
{
    return LoggerSeverityFilter<std::decay_t<T>>(sev, std::forward<T>(logger));
}

}

template<class L>
class LoggerFormatter : public Logger
{
public:
    LoggerFormatter(StrSpan format, L logger)
        : m_logger(std::move(logger))
        , m_format(format.data(), format.size())
    { }

    bool is_enabled(const Metadata& meta) override { return m_logger.is_enabled(meta); }

    void write(const Record& record) override
    {
        util::panic("Not implemented :(");
        // TODO: 
    }

private:
    L m_logger;
    std::string m_format;
};

namespace logger_formatter
{

template<class T>
LoggerFormatter<std::decay_t<T>> make(Str format, T&& logger)
{
    return LoggerFormatter<std::decay_t<T>>(format, std::forward<T>(logger));
}

}

} // namespace diag
} // namespace common
