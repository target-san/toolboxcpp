#include <ostream>
#include <sstream>
#include <type_traits>

/** @defgroup errors_impl Implementation details of errors handling infrastructure
 *  @{
 */
namespace toolboxcpp
{
namespace except
{
namespace impl
{
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
    struct FailureImplBase
    {
    protected:
        FailureImplBase()
            : _loc()
        { }
    
        FailureImplBase(util::SourceLocation loc)
            : _loc(loc)
        { }

        void display_loc(std::ostream& ost)
        {
            if(_loc.file && *_loc.file)
                ost << _loc.file;
            else
                ost << "<unknown>";
            if(_loc.line != 0)
                ost << ":" << _loc.line;
            ost << " (";
            if(_loc.func && *_loc.func)
                ost << _loc.func;
            else
                ost << "<unknown>";
            ost << ")";

        }

        const char* what_impl() const noexcept
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
        util::SourceLocation _loc;
        mutable std::string  _cache;
    };
    /** Wraps any structure and makes it derived from both std::exception and errors::Failure
     *  Such structure must have
     */
    template<typename E>
    struct StructFailure
        : public E
        , public Failure
        , public std::exception
    {
        AnyFailure(E const& that, util::SourceLocation loc)
            : E(that)
            , FailureImplBase(loc)
        { }

        AnyFailure(E&& that, util::SourceLocation loc)
            : E(std::move(that))
            , FailureImplBase(loc)
        { }

        void display(std::ostream& ost) const override
        {
            // TODO: type-based selector for E's brief, details etc.
            E::brief(ost);
            ost << "\n    at ";
            display_loc(ost);
            ost << "\n";
            E::details(ost);
        } 

        const char* what() const noexcept override
        {
            return what_impl();
        }
    };
    /** Wraps any class derived from std::exception to make it compatible with errors::Failure
     */
    template<typename E>
    struct ExceptionFailure
        : public E
        , public Failure
        , public FailureImplBase<ExceptionFailure<E>>
    {
        ExceptionFailure(E const& ex, util::SourceLocation loc)
            : E(ex)
            , FailureImplBase(loc)
        { }
        
        ExceptionFailure(E&& ex, util::SourceLocation loc)
            : E(std::move(ex))
            , FailureImplBase(loc)
        { }

        void display(std::ostream& ost) const override
        {
            ost << E::what() << "\n    at ";
            display_loc(ost);
        }

        const char* what() const noexcept override
        {
            return what_impl();
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
        enum_exception_chain(disp.ptr,
            [&] (util::FuncRef<void()> rethrow)
            {
                try
                {
                    rethrow();
                }
                catch(errors::Failure& e)
                {
                    ost << e << "\n";
                }
                catch(std::exception& e)
                {
                    ost << e.what() << "\n";
                }
                catch(...)
                {
                    ost << "Non-C++ exception\n";
                }
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
            ::errors::raise(std::forward<E>(error), loc);            
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
                ::errors::raise(error_fn(), loc);
            }
        }
    };
    
    template<typename E>
    ErrorContext<typename std::decay<E>::type> make_error_context(util::SourceLocation loc, E&& error_fn)
    {
        return ErrorContext<typename std::decay<E>::type> { loc, std::forward<E>(error_fn) };
    }

    /** Shows whole exception chain as exception's cause
     */
    struct DetailedFailure
        : public std::exception
        , public Failure
        , public FailureImplBase<DetailedFailure>
    {
        DetailedFailure(std::exception_ptr ptr)
            : ptr(ptr)
        { }

        void display(std::ostream& ost) const override
        {
            ost << display_exception(ptr);
        }

        const char* what() const noexcept override
        {
            return what_impl();
        }

    private:
        std::exception_ptr ptr;
    };
}//namespace impl

    [[noreturn]] void rethrow_with_detailed_cause(std::exception_ptr ex)
    {
        try
        {
            std::rethrow_exception(ex);
        }
        catch(std::nested_exception&)
        {
            throw impl::DetailedFailure(std::current_exception());            
        }
    }

    [[noreturn]] void rethrow_current_with_detailed_cause()
    {
        rethrow_with_detailed_cause(std::current_exception());
    }
/// @}

}//namespace errors
}//namespace toolboxcpp
