#pragma once

#include "../util/FuncRef.hpp"
#include "../util/SourceLocation.hpp"

#include <exception>
#include <ostream>
#include <sstream>
#include <type_traits>
/** Wraps provided error object so that it's compatible with both std::exception and errors::Failure
 *  and throws it
 *  If there's current exception present, whole object is thrown upwards
 *  using std::throw_with_nested, effectively capturing current exception as nested
 *
 *  @see ::errors::raise
 *
 *  Usage:
 *  @code
 *      $raise std::logic_error("Wrong behavior here, really");
 *  @endcode
 */
#define $raise ::errors::impl::make_raise_tag($SourceLocation) %= 
/** When any exception flies out of this context, a new exception is thrown
 *  and original one is placed inside as nested one
 *
 *  Usage:
 *  @code
 *  $error_context("Operation failure")
 *  {
 *      // some logic here
 *      // ...
 *      if (!condition)
 *          $raise DomainError { "description", error_code };
 *  };
 *  @endcode
 *
 *  @param $error Error object which is used as wrapper
 */
#define $error_context($error) ::errors::impl::make_error_context($SourceLocation, [&] () { return ($error); }) % [&] ()

namespace errors
{
    /** Generic failure interface
     *  Unlike std::exception, it's a passive displayer, i.e. it writes info to provided stream
     */
    struct Failure
    {
        virtual void display(std::ostream&) const = 0;
        virtual ~Failure() {}
    };
    
    inline std::ostream& operator << (std::ostream& ost, Failure const& fail)
    {
        fail.display(ost);
        return ost;
    }
    /** Wraps provided error object and throws it as exception
     *
     *  Provided error object is wrapped to make it compatible with errors::Failure and std::exception,
     *  and source location is added.
     *  If there's exception being currently handled, it's attached as nested exception
     *
     *  1. If object is derived from std::exception, it's wrapped so that its original cause is used as brief message,
     *     and location is added as second line
     *  2. If object is a string literal or std::string, it's wrapped as special exception, where string is used
     *     as cause, and location is added during display
     *  3. Otherwise, if object is of arbitrary type, it must satisfy following requirements
     *     a) Object should have instance method 'brief(std::ostream&)', which is used as short cause line
     *     b) Object may have instance method 'details(std::ostream&)', whose result is appended after location
     *     c) If object doesn't have 'brief' method, it may have 'display(std::ostream&)' as replacement
     *     d) If object doesn't have 'display' method, it may have stream output operator which works as replacement
     */
    template<typename E>
    [[noreturn]] void raise(E&& error, util::SourceLocation loc);

    namespace impl { struct ExceptionPtrDisplayer; /** Forward declaration of helper structure */}
    /** Wraps exception pointer into streamable object
     *
     *  Returned object can be output to any STL stream, in which case whole exception chain is output
     *  @param  ex  Exception pointer for output
     *  @return     Wrapper object which writes whole exception chain to any stream
     */
    impl::ExceptionPtrDisplayer display_exception(std::exception_ptr ex);
    /** Wraps current exception pointer into streamable object
     *
     *  Returned object can be output to any STL stream, in which case whole exception chain is output
     *  @return     Wrapper object which writes whole exception chain to any stream
     */
    impl::ExceptionPtrDisplayer display_current_exception();
    /** Walk whole chain of nested exceptions and invoke provided user's callback for each such exception
     *  User callback receives rethrow function, which, when called, will throw current exception in chain
     *  This is due to how exceptions are implemented in C++ - the only way to investigate them is to throw one
     *  and then catch with a proper catch block
     *  If exception isn't caught by callback, it's silently swallowed and doesn't leak to outer code
     *
     *  @param  ex          Exception pointer
     *  @param  callback    User callback which is called for each nested exception in chain
     */
    void enum_exception_chain(std::exception_ptr ex, util::FuncRef<void(util::FuncRef<void()> /*rethrow*/)> callback);
    /** Wraps exception so that it shows whole exception chain as cause
     *
     *  If exception provided has std::nested_exception as a mixin,
     *  it's wrapped such that it would return description of whole exception chain
     *  as its cause; otherwise, it's simply rethrown
     *  This is useful when you can't control top-level catch block, which simply
     *  uses std::exception::what
     *
     *  @param  ex  Exception being wrapped
     */
    [[noreturn]] void rethrow_with_detailed_cause(std::exception_ptr ex);
    [[noreturn]] void rethrow_current_with_detailed_cause();

/** @defgroup errors_impl Implementation details of errors handling infrastructure
 *  @{
 */
namespace impl
{
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
    struct AnyFailure
        : public E
        , public Failure
        , public std::exception
        , public FailureImplBase<AnyFailure<E>>
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
    /** Wraps std::string as failure
     */
    template<typename T = std::string> // To make it compatible with raise_impl
    struct StringFailure
        : public std::exception
        , public Failure
        , public FailureImplBase<StringFailure<T>>
    {
        StringFailure(std::string cause, util::SourceLocation loc)
            : _cause(std::move(cause))
            , FailureimplBase(loc)
        { }
        
        void display(std::ostream& ost) const override
        {
            ost << _cause << "\n    at ";
            display_loc(ost);
        }
        
        const char* what() const noexcept override
        {
            return what_impl();
        }
    
    private:
        std::string _cause;
    };
    /**
     *  @brief Raise exception, wrapping specified error object into one of impls
     *  Also, if there's currently exception being processed, 
     */
    template<template<class> typename W, typename E>
    [[noreturn]] void raise_impl(E&& error, util::SourceLocation loc)
    {
        using Err = typename std::decay<E>::type;
        if(std::current_exception())
            std::throw_with_nested(W<Err>(std::forward<E>(error), loc));
        else
            throw W<Err>(std::forward<E>(error), loc);
    }

    template<
        typename E,
        typename = typename std::enable_if<
            std::is_base_of<std::exception, typename std::decay<E>::type>::value
        >::type
    >        
    [[noreturn]] void raise(E&& error, util::SourceLocation loc)
    {
        raise_impl<ExceptionFailure>(std::forward<E>(error), loc);
    }

    template<
        typename E,
        typename = typename std::enable_if<
            !std::is_base_of<std::exception, typename std::decay<E>::type>::value
        >::type
    >        
    [[noreturn]] void raise(E&& error, util::SourceLocation loc)
    {
        raise_impl<AnyFailure>(std::forward<E>(error), loc);
    }
    
    [[noreturn]] void raise(std::string cause, util::SourceLocation loc)
    {
        raise_impl<StringFailure>(std::move(cause), loc);
    }

    [[noreturn]] void raise(const char* cause, util::SourceLocation loc)
    {
        raise_impl<StringFailure>(std::string(cause), loc);
    }
} // namespace errors::impl

    template<typename E>
    [[noreturn]]
    void raise(E&& error, util::SourceLocation loc)
    {
        impl::raise(std::forward<E>(error), loc);
    }
    
    inline void enum_exception_chain(std::exception_ptr ex, util::FuncRef<void(util::FuncRef<void()> /*rethrow*/)> callback)
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
    
    inline void enum_current_exception_chain(util::FuncRef<void(util::FuncRef<void()> /*rethrow*/)> callback)
    {
        enum_exception_chain(std::current_exception(), callback);
    }

namespace impl
{
    /// Helper structure which would dump whole exception chain into provided stream
struct ExceptionPtrDisplayer { std::exception_ptr ptr; };
    
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
}

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

}
