#pragma once

#include <toolboxcpp/util/FoldTuple.hpp>

#include <tuple>
#include <utility>

/**
    Implementation of default formatter which captures all passed in variables into object
    and dumps them into provided stream on-demand
*/

namespace toolboxcpp
{
namespace log
{

template<typename... Args>
struct DefaultFormatter
{
private:
    struct Write
    {
        template<typename T>
        std::ostream& operator()(std::ostream& ost, T&& arg)
        {
            ost << arg;
            return ost;
        }
    };

public:
    DefaultFormatter(Args... args)
        : _args(std::forward<Args>(args)...)
    { }
    
    void operator () (std::ostream& ost) const
    {
        util::fold_tuple(_args, ost, Write{});
    }

private:
    std::tuple<Args...> _args;
};

template<typename... Args>
DefaultFormatter<Args...> default_format(Args&&... args)
{
    return DefaultFormatter<Args...>(std::forward<Args>(args)...);
}

}
}
