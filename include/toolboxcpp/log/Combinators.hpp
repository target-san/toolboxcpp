#pragma once
/** Set of useful combinators and wrappers for constructing your own logger implementation
 *  Completely independent of any kind of concrete implementation
 */
#include <string>
#include <tuple>
#include <utility>
#include <sstream>

#include "../Log.hpp"
#include "Logger.hpp"
#include "../util/FoldTuple.hpp"

namespace toolboxcpp
{
namespace log
{
    /** Composes several loggers and sends message to each of them
     */
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
            util::fold_tuple(_loggers, 0, Write{ rec, writer });
        }

    private:
        std::tuple<Logs...> _loggers;
    };
    /** Construct multi-logger out of multiple nested loggers
     */
    template<class... Args>
    MultiLogger<typename std::decay<Args>::type...> make_multi_logger(Args&&... args)
    {
        return MultiLogger<typename std::decay<Args>::type...>(std::forward<Args>(args)...);
    }
    /** Logger which makes enabled/disabled decision based on call to unary functor
     *
     *  Filter functor should have signature compatible with
     *  @code
     *  bool (Metadata const&)
     *  @endcode
     */
    template<typename Fn, typename L>
    class FilteredLogger
    {
    public:
        FilteredLogger(Fn filter, L logger)
            : _filter(std::move(filter))
            , _logger(std::move(logger))
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
        Fn  _filter;
        L   _logger;
    };
    /** Construct filtered logger from fliter functor and nested logger
     */
    template<typename Fn, typename L>
    FilteredLogger<typename std::decay<Fn>::type, typename std::decay<L>::type>
    make_filtered_logger(Fn&& filter, L&& logger)
    {
        return FilteredLogger<typename std::decay<Fn>::type, typename std::decay<L>::type>
            (std::forward<Fn>(filter), std::forward<L>(logger));
    }
    /** Applies additional formatting to message using provided formatting functor
     *
     *  Format functor should have signature compatible with:
     *  @code
     *  void (std::ostream&, Record const&, WriterFunc)
     *  @endcode
     */
    template<typename Fn, typename L>
    class FormattedLogger
    {
    public:
        FormattedLogger(Fn formatter, L logger)
            : _formatter(std::move(formatter))
            , _logger(std::move(logger))
        { }

        bool is_enabled(Metadata const& meta) { return _logger.is_enabled(meta); }

        void write(Record const& rec, WriterFunc writer)
        {
            auto format_proxy = [&] (std::ostream& ost) { _formatter(ost, rec, writer); };
            _logger.write(rec, format_proxy);
        }

    private:
        Fn  _formatter;
        L   _logger;
    };
    /** Construct formatted logger out of filter functor and nested logger
     */
    template<typename Fn, typename L>
    FormattedLogger<typename std::decay<Fn>::type, typename std::decay<L>::type>
    make_formatted_logger(Fn&& formatter, L&& logger)
    {
        return FormattedLogger<typename std::decay<Fn>::type, typename std::decay<L>::type>
            (std::forward<Fn>(formatter), std::forward<L>(logger));
    }
    /** Writes whole message into intermediate buffer and then sends that buffer to wrapped logger as messafe
     *
     *  This can be useful when you know that certain message will be written to multiple underlying streams.
     *  In this case, performing formatting once may save you some time.
     */
    template<typename L>
    struct CachedLogger
    {
    public:
        CachedLogger(L logger)
            : _logger(std::forward<L>(logger))
        { }

        bool is_enabled(Metadata const& meta)
        {
            return _logger.is_enabled(meta);
        }

        void write(Record const& rec, WriterFunc writer)
        {
            std::ostringstream msg;
            writer(msg);
            auto writer_proxy = [&](std::ostream& ost) { ost << msg.rdbuf(); };
            _logger.write(rec, writer_proxy);
        }
    private:
        L _logger;
    };
    /** Constructs cached logger by wrapping another logger
     */
    template<typename L>
    CachedLogger<typename std::decay<L>::type> make_cached_logger(L&& logger)
    {
        return CachedLogger<typename std::decay<L>::type>(std::forward<L>(logger));
    }
} // namespace logger
} // namespace log_facade
