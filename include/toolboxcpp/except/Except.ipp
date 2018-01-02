#include <ostream>
#include <sstream>
#include <type_traits>
#include <type_info>

/** @defgroup errors_impl Implementation details of errors handling infrastructure
 *  @{
 */
namespace toolboxcpp
{
namespace except
{
namespace impl
{
    // Declare expression validity checker for certain type
    // Haven't found generic way to check if arbitrary expression is valid
#   define $DeclareIsXDefinedFor($name, $expr)                      \
    template<typename T, typename = void>                           \
    struct Is ## $name ## DefinedFor : public std::false_type {};   \
    template<typename T>                                            \
    struct Is ## $name ## DefinedFor<T, decltype(($expr, void()))>  \
        : public std::true_type {};                                 \
    /**/

    $DeclareIsXDefinedFor(MethodMessage, std::declval<typename std::decay<T>::type const&>().message(std::declval<std::ostream&>()))
    $DeclareIsXDefinedFor(FuncMessage,   message(std::declval<std::ostream&>(), std::declval<typename std::decay<T>::type const&>()))

    $DeclareIsXDefinedFor(MethodDetails, std::declval<typename std::decay<T>::type const&>().details(std::declval<std::ostream&>()))
    $DeclareIsXDefinedFor(FuncDetails,   details(std::declval<std::ostream&>(), std::declval<typename std::decay<T>::type const&>()))

#   undef  $DeclareIsXDefinedFor

    template<size_t I>
    using Index = std::integral_constant<size_t, I>;
    // Helper struct, returns amount of remaining bools when 1st of them is true
    // Effectively returns "inverted" index of 1st true value
    template<bool... Bs> struct BoolPriority;
    template<bool... Bs> struct BoolPriority<true, Bs...> : public Index<sizeof...(Bs) + 1u> {};
    template<>           struct BoolPriority<>            : public Index<0> {};
    template<bool... Bs> struct BoolPriority<false, Bs...>: public BoolPriority<Bs...> {};

    template<typename E>
    void display_message_impl(std::ostream& ost, E const& error, Index<2>)
    {
        error.message(ost);
    }

    template<typename E>
    void display_message_impl(std::ostream& ost, E const& error, Index<1>)
    {
        message(ost, error);
    }

    template<typename E>
    void display_message_impl(std::ostream& ost, E const& error, Index<0>)
    {
        ost << typeid(E).name();
    }

    template<typename E>
    void display_details_impl(std::ostream& ost, E const& error, Index<2>)
    {
        ost << "\n";
        error.details(ost);
    }

    template<typename E>
    void display_details_impl(std::ostream& ost, E const& error, Index<1>)
    {
        ost << "\n";
        details(ost, error);
    }

    template<typename E>
    void display_details_impl(std::ostream& ost, E const& error, Index<0>)
    { }

    template<typename E>
    void display_message(std::ostream& ost, E const& error)
    {
        display_message_impl(ost, error,
            BoolPriority<
                IsMethodMessageDefinedFor<E>::value,
                IsFuncMessageDefinedFor<E>::value
            >()
        );
    }

    template<typename E>
    void display_details(std::ostream& ost, E const& error)
    {
        display_details_impl(ost, error,
            BoolPriority<
                IsMethodDetailsDefinedFor<E>::value,
                IsFuncDetailsDefinedFor<E>::value
            >()
        );
    }

    inline void display_location(std::ostream& ost, ::toolboxcpp::util::SourceLocation loc)
    {
        if(loc.file && *loc.file)
            ost << loc.file;
        else
            ost << "<unknown>";
        if(loc.line != 0)
            ost << ":" << loc.line;
        if(loc.func && *loc.func)
            ost << " (" << loc.func << ")";
    }
    
    template<typename D>
    struct WhatCache
    {
    protected:
        const char* what_cached() const noexcept
        {
            if(_cache.empty())
            {
                std::ostringstream ost;
                ((D*)this)->display(ost);
                _cache = ost.str();
            }
            return _cache.c_str();
        }
    private:
        mutable std::string  _cache;
    };
    // Keeps location and provides ways to print it
    struct DisplayLoc
    {
    protected:
        DisplayLoc(::toolboxcpp::util::SourceLocation loc)
            : _loc(loc)
        { }

        void display_loc(std::ostream& ost)
        {
            ost << "\n    at ";
            display_location(ost, _loc);
        }

    private:
        ::toolboxcpp::util::SourceLocation _loc;
    };
    /**
    Use E::message and E::details to implement both Failure::display and std::exception::what
    */
    template<typename E>
    struct StructFailure
        : public E
        , public Failure
        , public std::exception
        , public WhatCache<StructFailure<E>>
        , public DisplayLoc
    {
        StructFailure(E const& that, util::SourceLocation loc)
            : E(that)
            , DisplayLoc(loc)
        { }

        StructFailure(E&& that, util::SourceLocation loc)
            : E(std::move(that))
            , DisplayLoc(loc)
        { }

        void display(std::ostream& ost) const override
        {
            display_message<E>(ost, *this);
            display_loc(ost);
            display_details<E>(ost, *this);
        } 

        const char* what() const noexcept override
        {
            return what_cached();
        }
    };
    // Use std::exception::what provided by E to implement Failure::display
    template<typename E>
    struct StdFailure
        : public E
        , public Failure
        , public WhatCache<StdFailure<E>>
        , public DisplayLoc
    {
        StdFailure(E const& ex, ::toolboxcpp::util::SourceLocation loc)
            : E(ex)
            , DisplayLoc(loc)
        { }
        
        StdFailure(E&& ex, ::toolboxcpp::util::SourceLocation loc)
            : E(std::move(ex))
            , DisplayLoc(loc)
        { }

        void display(std::ostream& ost) const override
        {
            ost << E::what();
            display_loc(ost);
        }

        const char* what() const noexcept override
        {
            return what_cached();
        }
    };
    // Use Failure::display provided by E to implement std::exception::what
    template<typename E>
    struct ExceptFailure
        : public E
        , public std::exception
        , public WhatCache<ExceptFailure<E>>
    {
        ExceptFailure(E const& error)
            : E(error)
        { }

        ExceptFailure(E&& error)
            : E(std::move(error))
        { }

        ExceptFailure(ExceptFailure const&) = default;
        ExceptFailure(ExceptFailure &&)     = default;

        ExceptFailure& operator=(ExceptFailure const&) = default;
        ExceptFailure& operator=(ExceptFailure &&)     = default;

        const char* what() const noexcept override
        {
            return what_cached();
        }
    };

    template<typename E>
    auto except_type(const E*, const void*, const void*)
        -> StructFailure<typename std::decay<E>::type>;

    template<typename E>
    auto except_type(const E*, const std::exception*, const void*)
        -> StdFailure<typename std::decay<E>::type>;

    template<typename E>
    auto except_type(const E*, const void*, const ::toolboxcpp::except::Failure*)
        -> ExceptFailure<typename std::decay<E>::type>;

    template<typename E>
    auto except_type(const E*, const std::exception*, const ::toolboxcpp::except::Failure*)
        -> typename std::decay<E>::type;

    template<typename F, typename E>
    F make_failure(E&& error, ::toolboxcpp::util::SourceLocation loc, const void*)
    {
        return F(std::forward<E>(error), loc);
    }

    template<typename F, typename E>
    F make_failure(E&& error, ::toolboxcpp::util::SourceLocation, const ::toolboxcpp::util::Failure*)
    {
        return F(std::forward<E>(error));
    }
} // namespace errors::impl

    template<typename E>
    [[noreturn]]
    void raise_at(E&& error, ::toolboxcpp::util::SourceLocation loc)
    {
        using Fail = decltype(impl::except_type(&error, &error, &error));
        auto fail = impl::make_failure<Fail>(std::forward<E>(error), loc, &error)
        if(std::current_exception())
            std::throw_with_nested(std::move(fail));
        else
            throw fail;
    }
    
    template<typename Fn>
    void enum_exception_chain(std::exception_ptr ex, Fn&& callback)
    {
        while(ex != nullptr)
        {
            std::exception_ptr current;
            // First, dismantle into current and nested - if possible
            try
            {
                std::rethrow_exception(ex);
            }
            catch(std::nested_exception& outer)
            {   // Swap 'ex' into current, for later use, and capture nested into 'ex'
                current = ex;
                ex = outer.nested_ptr(); // Capture nested, for later use
            }
            catch(...)
            {   // If 'ex' is not a nested_exception, recapture it into 'current' and set 'ex' to nullptr, such that there will be no nested exception
                current = ex;
                ex = nullptr; // There will be no nested
            }
            // Just in case
            if(current == nullptr)
            {
                break;
            }
            // Next, pass whole stuff to callback
            try // Just in case someone would forget to catch current exception
            {
                callback([&]() { std::rethrow_exception(current); });
            }
            catch(...) // In this particular case, swallow anything flying outside
            { }
        }
    }

namespace impl
{
    /// Helper structure which would dump whole exception chain into provided stream
    struct ExceptionPtrDisplayer
    {
        std::exception_ptr ptr;
    };
    
    inline std::ostream& operator << (std::ostream& ost, ExceptionPtrDisplayer const& disp)
    {
        static constexpr const char INITIAL[] = "Exception: ", FOLLOW = "Caused by: ";
        const char* prefix = INITIAL;
        enum_exception_chain(disp.ptr,
            [&] (util::FuncRef<void()> rethrow)
            {
                ost << prefix;
                try
                {
                    rethrow();
                }
                catch(errors::Failure& e)
                {
                    ost << e;
                }
                catch(std::exception& e)
                {
                    ost << e.what();
                }
                catch(...)
                {
                    ost << "Non-C++ exception";
                }
                ost << "\n";
                prefix = FOLLOW;
            }
        );
        return ost;
    }
}
    
    inline impl::ExceptionPtrDisplayer display_exception(std::exception_ptr ex)
    {
        return impl::ExceptionPtrDisplayer { ex };
    }
    
    inline impl::ExceptionPtrDisplayer display_current_exception()
    {
        return display_exception(std::current_exception());
    }

namespace impl
{
    struct RaiseTag
    {
        util::SourceLocation loc;
        
        template<typename E>
        [[noreturn]] void operator %= (E&& error)
        {
            ::toolboxcpp::except::raise_at(std::forward<E>(error), loc);            
        }
    };
    
    RaiseTag make_raise_tag(util::SourceLocation loc)
    {
        return RaiseTag { loc };
    }
    
    template<typename ErrFn>
    struct ErrorContext
    {
        util::SourceLocation loc;
        ErrFn                error_fn;
        
        template<typename Fn>
        auto operator % (Fn&& body) -> decltype(body())
        {
            try
            {
                return body();
            }
            catch(...)
            {
                ::toolboxcpp::except::raise_at(error_fn(), loc);
            }
        }
    };
    
    template<typename E>
    ErrorContext<typename std::decay<E>::type> make_error_context(util::SourceLocation loc, E&& error_fn)
    {
        return ErrorContext<typename std::decay<E>::type> { loc, std::forward<E>(error_fn) };
    }

}//namespace impl

/// @}

}//namespace errors
}//namespace toolboxcpp
