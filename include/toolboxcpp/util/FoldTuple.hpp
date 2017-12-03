#pragma once
#include <type_traits>
#include <utility>
/** Templated fold function over tuples of arbitrary arity
*/

namespace toolboxcpp
{
namespace util
{

namespace impl
{
    template<size_t I, size_t N> struct Folder;

    template<size_t I, size_t N>
    struct Folder
    {
        template<typename Tuple, typename Acc, typename Func>
        static auto apply(Tuple&& tuple, Acc&& acc, Func&& func)
            -> decltype(
                Folder<I+1, N>::apply(
                    std::forward<Tuple>  (tuple),
                    func(
                        std::forward<Acc>(acc),
                        std::get<I>(std::forward<Tuple>(tuple))
                    ),
                    std::forward<Func>   (func)
                )
            )
        {
            // Compute type of next accumulator
            // Here, we depend on the fact that std::get over rvalue tuple
            // will not move whole tuple, just create rvalue ref over tuple element
            // and not perform actual move
            using NextAcc = decltype(func( std::forward<Acc>(acc), std::get<I>(std::forward<Tuple>(tuple)) ));
            // Used to pre-capture forwarding references to tuple and func, such that next accumulator
            // is computed before next step
            struct Applier
            {
                Tuple&& tuple;
                Func&&  func;

                auto apply(NextAcc&& acc)
                    -> decltype(
                        Folder<I+1, N>::apply(
                            std::forward<Tuple>  (tuple),
                            std::forward<NextAcc>(acc),
                            std::forward<Func>   (func)
                        )
                    )
                {
                    return Folder<I+1, N>::apply(
                        std::forward<Tuple>  (tuple),
                        std::forward<NextAcc>(acc),
                        std::forward<Func>   (func)
                    );
                }
            };
            
            return (Applier { std::forward<Tuple>(tuple), std::forward<Func>(func) })
                .apply(func(
                    std::forward<Acc>(acc),
                    std::get<I>(std::forward<Tuple>(tuple))
                ));
        }
    };
    // Terminating specialization
    template<size_t N>
    struct Folder<N, N>
    {
        template<typename Tuple, typename Acc, typename Func>
        static auto apply(Tuple&&, Acc&& acc, Func&&)
            -> Acc
        {
            return acc;
        }
    };
}

/** @brief Perform fold over all elements of any tuple-like object

    For each element of tuple, invoke 'folder' functor with accumulator as first argument
    and tuple element as second one, then use returned value as new accumulator and repeat
    over the rest of tuple; return final accumulator value in the end
    
    Please note that accumulator may change its type during folding, based on which overload
    of binary functor 'folder' is called
    
    @tparam Tuple       Type of tuple-like object
    @tparam Acc         Initial accumulator's type
    @tparam Func        Fold functor's type
    
    @param  tuple       Tuple-like object, being folded
    @param  accumulator Initial accumulator value
    @param  folder      Fold functor
    
    @return             Final accumulator value, produced by last operation
*/
template<typename Tuple, typename Acc, typename Func>
auto fold_tuple(Tuple&& tuple, Acc&& accumulator, Func&& folder)
    // Note: consider return value type as implementation-defined
    -> decltype(impl::Folder<0, std::tuple_size<typename std::decay<Tuple>::type>::value>
        ::apply(
            std::forward<Tuple> (tuple),
            std::forward<Acc>   (accumulator),
            std::forward<Func>  (folder)
        )
    )
{
    return impl::Folder<0, std::tuple_size<typename std::decay<Tuple>::type>::value>
        ::apply(
            std::forward<Tuple> (tuple),
            std::forward<Acc>   (accumulator),
            std::forward<Func>  (folder)
        );
}

}
}
