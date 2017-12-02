#pragma once

#include <type_traits>
#include <cassert>

namespace toolboxcpp
{
namespace util
{
/**
 *  @brief Non-owning reference to any callable object, be it free function or functor
 *
 *  Abstracts out any callable object without owning it or moving it to heap
 *  Useful when need to pass callback through translation unit boundary
 *  without exposing its internals
 *
 *  !!!WARN!!! Does not prolong lifetime of wrapped object. So please keep watch of the lambdas lifetimes
 *
 *  NB: only functors and normal free functions are supported. Use std::bind or lambdas to wrap member functions
 *
 *  @tparam Signature Call signature, like with std::function
 */
template<class Signature> class FuncRef;
/**
 *  @brief Non-owning reference to any callable object, be it free function or functor
 *
 *  Abstracts out any callable object without owning it or moving it to heap
 *  Useful when need to pass callback through translation unit boundary
 *  without exposing its internals
 *
 *  !!!WARN!!! Does not prolong lifetime of wrapped object. So please keep watch of the lambdas lifetimes
 *
 *  NB: only functors and normal free functions are supported. Use std::bind or lambdas to wrap member functions
 *
 *  @tparam R   Return type
 *  @tparam Ts  Argument types
 */
template<class R, class... Ts>
class FuncRef<R(Ts...)>
{
public:
    using Pointer  = R(*)(Ts...);
    using Callback = R(*)(void*, Ts...);
    /** @brief Wraps any compatible callable object
    */
    template<class Fn>
    FuncRef(Fn&& func)
        : _context(reinterpret_cast<void*>(&func))
        , _caller(&ObjectCaller<typename std::decay<Fn>::type>)
    { }
    /** @brief Wraps any compatible free function
    */
    FuncRef(Pointer func)
        : _context(reinterpret_cast<void*>(func))
        , _caller(&FuncCaller)
    {
        assert(_context);
    }

    FuncRef(FuncRef const&) = default;
    FuncRef& operator= (FuncRef const&) = default;
    /** @brief Invoke wrapped callable
    */
    R operator () (Ts... args) const
    {
        return (*_caller)(_context, std::forward<Ts>(args)...);
    }
    /**
     * @brief Returns C-compatible callback which accepts context as first argument
     * @return
     */
    Callback callback() const noexcept
    {
        return &CallbackCaller;
    }
    /**
     * @brief Returns C-compatible context pointer, intended to be used with function pointer returned by callback()
     * @return
     */
    void*    context() noexcept
    {
        return this;
    }

private:
    using Caller = R(*)(void*, Ts&&... args);

    void*    _context;
    Caller   _caller;

    template<class Fn>
    static R ObjectCaller(void* object, Ts&&... args)
    {
        return (*reinterpret_cast<Fn*>(object))(std::forward<Ts>(args)...);
    }

    static R FuncCaller(void* func, Ts&&... args)
    {
        return (reinterpret_cast<Pointer>(func))(std::forward<Ts>(args)...);
    }

    static R CallbackCaller(void* context, Ts... args)
    {
        FuncRef<R(Ts...)>* funcref = (FuncRef<R(Ts...)>*)context;
        return (funcref->_caller)(funcref->_context, std::forward<Ts>(args)...);
    }
};

}
}
