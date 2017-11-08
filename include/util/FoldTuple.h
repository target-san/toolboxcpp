#include <tuple>
#include <utility>
/** Templated fold function over tuples of arbitrary arity
*/

namespace util
{

namespace impl
{
/** Terminating version of fold implementation
    Simply forward-returns accumulator
*/
template<typename Tuple, typename Acc, typename Fn>
decltype(auto) fold_impl(Tuple&&, Acc&& acc, Fn&&, std::index_sequence<>)
{
    return std::forward<Acc>(acc);
}
/** Generic fold implementation
    1. Call folder func over accumulator and I'th tuple element
    2. Recursive call over tuple, new accumulator, folder func and the rest of index sequence
*/
template<typename Tuple, typename Acc, typename Fn, size_t I, size_t Is...>
decltype(auto) fold_impl(Tuple&& tuple, Acc&& acc, Fn&& func, std::index_sequence<I, Is...>)
{
    decltype(auto) next = func(std::forward<Acc>(acc), std::get<I>(std::forward<Tuple>(tuple)));
    return fold_impl(
        std::forward<Tuple>(tuple),
        std::forward<decltype(next)>(next),
        std::forward<Fn>(func),
        std::index_sequence<Is...>()
    );        
}

}

/** @brief Perform fold over all elements of tuple, in sequence

    For each element of tuple, invole 'folder' functor with accumulator as first argument
    and tuple element as second one, then use returned value as new accumulator and repeat
    over the rest of tuple; return final accumulator value in the end
    
    Please note that accumulator may change its type during folding, based on which overload
    of binary functor 'folder' is called
    
    @tparam Ts          Tuple element types
    @tparam Acc         Initial accumulator's type
    @tparam Fn          Fold functor's type
    
    @param  tuple       Actual tuple being folded
    @param  accumulator Initial accumulator value
    @param  folder      Fold functor
    
    @return             Final accumulator value, produced by last operation
*/
template<typename... Ts, typename Acc, typename Fn>
decltype(auto) fold_tuple(std::tuple<Ts...> const& tuple, Acc&& accumulator, Fn&& folder)
{
    return impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), std::index_sequence_for<Ts...>);
}

template<typename... Ts, typename Acc, typename Fn>
decltype(auto) fold_tuple(std::tuple<Ts...>& tuple, Acc&& accumulator, Fn&& folder)
{
    return impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), std::index_sequence_for<Ts...>);
}

template<typename... Ts, typename Acc, typename Fn>
decltype(auto) fold_tuple(std::tuple<Ts...>&& tuple, Acc&& accumulator, Fn&& folder)
{
    return impl::fold_impl(std::move(tuple), std::forward<Acc>(accumulator), std::forward<Fn>(folder), std::index_sequence_for<Ts...>);
}

template<typename... Ts, typename Acc, typename Fn>
decltype(auto) fold_tuple(std::tuple<Ts...> tuple, Acc&& accumulator, Fn&& folder)
{
    return impl::fold_impl(std::move(tuple), std::forward<Acc>(accumulator), std::forward<Fn>(folder), std::index_sequence_for<Ts...>);
}

}
