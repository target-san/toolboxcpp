#pragma once
#include <tuple>
#include <utility>
/** Templated fold function over tuples of arbitrary arity
*/

namespace log_facade
{
namespace util
{

namespace impl
{
/** Need to reimplement index_sequence for C++11 since it's only in C++14
 */
template<size_t... Is> struct IndexSeq { static constexpr size_t Size = sizeof...(Is); };

template<typename S1, typename S2> struct ConcatSeq;
template<size_t... Is1, size_t... Is2>
struct ConcatSeq<IndexSeq<Is1...>, IndexSeq<Is2...>> { using Type = IndexSeq<Is1..., Is2...>; };

template<typename S, size_t N> struct OffsetSeq;
template<size_t... Is, size_t N>
struct OffsetSeq<IndexSeq<Is...>, N> { using Type = IndexSeq<(Is+N)...>; };

template<size_t N>
struct MakeIndexSeqImpl
{
    using Type = typename ConcatSeq<
        typename MakeIndexSeqImpl<N/2>::Type,
        typename OffsetSeq<typename MakeIndexSeqImpl<N - N/2>::Type, N/2>::Type
    >::Type;
};

template<> struct MakeIndexSeqImpl<0> { using Type = IndexSeq<>; }; 
template<> struct MakeIndexSeqImpl<1> { using Type = IndexSeq<0>; };

template<size_t N>
using MakeIndexSeq = typename MakeIndexSeqImpl<N>::Type;
template<typename... Ts>
using MakeIndexSeqFor = MakeIndexSeq<sizeof...(Ts)>;
/** Terminating version of fold implementation
    Simply forward-returns accumulator
*/
template<typename Tuple, typename Acc, typename Fn>
auto fold_impl(Tuple&&, Acc&& acc, Fn&&, IndexSeq<>) -> decltype(std::forward<Acc>(acc))
{
    return std::forward<Acc>(acc);
}
/** Generic fold implementation
    1. Call folder func over accumulator and I'th tuple element
    2. Recursive call over tuple, new accumulator, folder func and the rest of index sequence
*/
template<typename Tuple, typename Acc, typename Fn, size_t I, size_t... Is>
auto fold_impl(Tuple&& tuple, Acc&& acc, Fn&& func, IndexSeq<I, Is...>)
    -> decltype(fold_impl(
        std::forward<Tuple>(tuple),
        func(std::forward<Acc>(acc), std::get<I>(tuple)),
        std::forward<Fn>(func),
        IndexSeq<Is...>()
    ))
{
    auto&& next = func(std::forward<Acc>(acc), std::get<I>(tuple));
    return fold_impl(
        std::forward<Tuple>(tuple),
        std::forward<decltype(next)>(next),
        std::forward<Fn>(func),
        IndexSeq<Is...>()
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
template<typename Acc, typename Fn, typename... Ts>
auto fold_tuple(std::tuple<Ts...> const& tuple, Acc&& accumulator, Fn&& folder)
    -> decltype(impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{}))
{
    return impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{});
}

template<typename Acc, typename Fn, typename... Ts>
auto fold_tuple(std::tuple<Ts...>& tuple, Acc&& accumulator, Fn&& folder)
    -> decltype(impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{}))
{
    return impl::fold_impl(tuple, std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{});
}

template<typename Acc, typename Fn, typename... Ts>
auto fold_tuple(std::tuple<Ts...>&& tuple, Acc&& accumulator, Fn&& folder)
    -> decltype(impl::fold_impl(std::move(tuple), std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{}))
{
    return impl::fold_impl(std::move(tuple), std::forward<Acc>(accumulator), std::forward<Fn>(folder), impl::MakeIndexSeqFor<Ts...>{});
}

}
}
