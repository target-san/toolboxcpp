#include <tuple>
#include <utility>
#include "../util/FoldTuple.h"
/**
    Implementation of default formatter which captures all passed in variables into object
    and dumps them into provided stream on-demand
*/

namespace diag
{
namespace impl
{

template<typename T>
struct RefArgWrap
{
    RefArgWrap(T& ref)
        : _ptr(&ref)
    { }    

    T& operator* () const noexcept { return *_ptr; }
            
private:
    T* _ptr;
};

template<typename T>
struct ValArgWrap
{
    ValArgWrap(T&& rref)
        : _ptr(std::move(rref))
    { }        

    T& operator* () const noexcept { return _val; }
            
private:
    T _val;
};

template<typename T> struct ArgWrap;
template<typename T> struct ArgWrap<T&>         : public RefArgWrap<T>       {};
template<typename T> struct ArgWrap<T const&>   : public RefArgWrap<const T> {};
template<typename T> struct ArgWrap<T>          : public ValArgWrap<T>       {};
template<typename T> struct ArgWrap<T&&>        : public ValArgWrap<T>       {};

template<typename... Args>
struct DefaultFormatter
{
public:
    DefaultFormatter(Args... args)
        : _args(std::forward<Args>(args)...)
    { }
    
    void operator () (std::ostream& ost)
    {
        util::foldTuple(_args, 0, [&ost] (int, auto&& arg) { ost << *arg; return 0; });
    }

private:
    std::tuple<ArgWrap<Args>...> _args;
};

template<typename... Args>
DefaultFormatter<Args...> defaultFormat(Args&&... args)
{
    return DefaultFormatter<Args...>(std::forward<Args>(args)...);
}

}
}
