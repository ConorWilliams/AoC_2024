#ifndef B87A46A7_5DC3_479A_823A_01D3F94B70B4
#define B87A46A7_5DC3_479A_823A_01D3F94B70B4

#include <concepts>
#include <type_traits>

#include "yeti/core/blessed.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"
#include "yeti/core/parser_obj.hpp"

namespace yeti {

/**
 * @brief The recursive extension of `parser_obj`.
 *
 * Concepts cannot be recursive, so this must be specified the old
 * fashioned way. This is then partially duplicated in concept-land
 * for be better error messages.
 *
 */
namespace impl::parser_concept {

template <typename P>
concept self_skip = same_as_stripped<P, skip_result_t<P>>;

template <typename P>
concept self_mute = same_as_stripped<P, mute_result_t<P>>;

// `T` unless all `S` are `void`, then `void`.
template <typename T, typename... S>
using unless_void = std::conditional_t<(std::is_void_v<S> && ...), void, T>;

// If `T` is `never` then `never` else `unit`
template <typename T>
using never_else_unit = std::conditional_t<std::same_as<T, never>, never, unit>;

/**
 * @brief Recursive implementation of the parser concept.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
struct parser_impl : std::false_type {};

// If neither P or S know the stream type then don't check for unit/never
// P = the mute parser, S is the stream it must parse, E == error of parent
// If E == never then propagate, else require unit.
template <typename P, typename S, typename T, typename E>
using mute_recur_help =
    parser_impl<P, S, T, unless_void<never_else_unit<E>, S, type_of<P>>>;

// Propagate static type into recursive call
// We remove the && to prevent `std::copy_constructible` causing the
// `parser_fn` concept to trigger.
template <typename P, typename S, typename T, typename E>
using mute_recur =
    mute_recur_help<remove_rvalue_t<mute_result_t<P>>, else_static<S, P>, T, E>;

// If neither P or S know the stream type then don't check for unit.
template <typename P, typename S, typename T, typename E>
using skip_recur_help =
    parser_impl<P, S, unless_void<never_else_unit<T>, S, type_of<P>>, E>;

// Propagate static type into recursive call
// We remove the && to prevent `std::copy_constructible` causing the
// `parser_fn` concept to trigger.
template <typename P, typename S, typename T, typename E>
using skip_recur =
    skip_recur_help<remove_rvalue_t<skip_result_t<P>>, else_static<S, P>, T, E>;

// Terminal case mute and skip OK
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_mute<P> && self_skip<P>
struct parser_impl<P, S, T, E> : std::true_type {};

// Skip OK check mute
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_skip<P>
struct parser_impl<P, S, T, E> : mute_recur<P, S, T, E> {};

// Mute OK check skip
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_mute<P>
struct parser_impl<P, S, T, E> : skip_recur<P, S, T, E> {};

// Neither OK, recurse both
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E>
struct parser_impl<P, S, T, E>
    : std::conjunction<mute_recur<P, S, T, E>, skip_recur<P, S, T, E>> {};

/**
 * @brief The full canonical definition of a parser.
 *
 * This implementation fully subsumes and additional checks in
 * the `parser` concept, they are there to provide better error
 * messages.
 */
template <typename P, typename S = void, typename T, typename E>
concept canonical_parser = parser_impl<P, S, T, E>::value;

// ===  === //
// ===  === //
// ===  === //

template <typename P, typename S, typename T, typename E>
concept core_parser_skip =
    parser_obj<remove_rvalue_t<P>,                             //
               S,                                              //
               unless_void<never_else_unit<T>, S, type_of<P>>, //
               E                                               //
               >;                                              //

template <typename P, typename S, typename T, typename E>
concept core_parser_mute =
    parser_obj<remove_rvalue_t<P>,                            //
               S,                                             //
               T,                                             //
               unless_void<never_else_unit<E>, S, type_of<P>> //
               >;                                             //

template <typename P, typename S, typename T, typename E>
concept parser_help =
    core_parser_skip<skip_result_t<P>, else_static<S, P>, T, E> &&
    core_parser_mute<mute_result_t<P>, else_static<S, P>, T, E> &&
    canonical_parser<P, S, T, E>;

/**
 * @brief Yeti's defining concept, the parser.
 *
 * @tparam P The parser type.
 * @tparam S The stream type over which the parser operates.
 * @tparam T The value type which the parser produces.
 * @tparam E The error type which the parser produces.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept parser = parser_obj<P, S, T, E> &&                                    //
                 parser_help<P, S, parse_value_t<P, S>, parse_error_t<P, S>>; //

} // namespace impl::parser_concept

using impl::parser_concept::parser;

/**
 * @brief Test if `P` is a parser over `Q`s stream type and returns the same
 * value and error types.
 *
 * This only requires that `Q` is a parser_fn.
 */
template <typename P, typename Q>
concept parser_like =
    parser_fn<Q>                                                  //
    && parser<P, type_of<Q>, parse_value_t<Q>, parse_error_t<Q>>; //

} // namespace yeti

#endif /* B87A46A7_5DC3_479A_823A_01D3F94B70B4 */
