#pragma once
/**
This header provides several basic tools for error handling

The main idea is to use exceptions as main means of error handling.
We could use Haskell-like result types but C++ cannot provide convenient
syntax standard-wise, and expression statements are available
only on GCC-compatible compilers. Although, Result-like way may be implemented
separately in future.

First of all, when we create exception hierarchies, we don't care about
std::exception as base class - we need only data. But we're forced to implement
virtual methods etc. for our exception being viewable uniformly.
So, the idea is for user to define only struct with necessary data fields
and 1 or 2 methods to display it, without need to explicitly derive
from any base type.

Second, it's very convenient to not only see where initial exception originated,
but also see in which context this happened, possibly with additional info.
So, it's more useful to wrap exception being bubbled into another exception
rather than keeping some kind of "stacktrace vector". Luckily, C++11 added
proper tools for exception nesting out-of-box, and we use it.

Third, std::exception::what as a way to print exception isn't the most flexible.
It usually requires to allocate separate buffer - usually by creating std::ostringstream,
which heavily impacts performance. As means to mitigate this, we create new interface,
which provides displayer method, which in its turn writes all necessary info to provided std::ostream

Fourth, it's very convenient to add place where exception was thrown, which info we add
upon wrapping initial exception structure.

Fifth, it's not very convenient to first display throw location lile boost::Exception does,
and then display lengty error information. So, instead, we consider format with first line being
short description, second being location, and the rest being exception details, if available.

This header provides following facilities:


*/

#include "../util/SourceLocation.hpp"

#include <exception>
#include <ostream>
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
#define $raise ::toolboxcpp::except::impl::make_raise_tag($SourceLocation) %= 
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
 *  }; // hidden lambda, so requires semicolon at the end
 *  @endcode
 *
 *  @param $error Error object which is used as wrapper
 */
#define $error_context($error) ::toolboxcpp::except::impl::make_error_context($SourceLocation, [&] () { return ($error); }) % [&] ()

namespace toolboxcpp
{
namespace except
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
    /**
    Wraps provided error object and throws it as exception

    Error object can be any type. Function produces new error object,
    derived from provided type, std::exception and toolboxcpp::except::Failure

    If source error object is derived from either std::exception or toolboxcpp::except::Failure,
    no additional inheritance is introduced for respective base type.

    @param  error   Error object being wrapped and thrown
    @param  loc     Location in sources where exception was raised
    */
    template<typename E>
    [[noreturn]] void raise_at(E&& error, ::toolboxcpp::util::SourceLocation loc);

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
     *  @tparam Fn          Type of callback functor
     *
     *  @param  ex          Exception pointer
     *  @param  callback    User callback which is called for each nested exception in chain
     *                      Signature: void (void() rethrow)
     *                      where rethrow: implementation-defined callback which will re-throw
     *                      current exception in enumerated chain
     */
    template<typename Fn>
    void enum_exception_chain(std::exception_ptr ex, Fn&& callback);

}//namespace except
}//namespace toolboxcpp

#include "Except.ipp" //< Implementation part
