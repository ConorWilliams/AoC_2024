#ifndef B87A46A7_5DC3_479A_823A_01D3F94B70B4
#define B87A46A7_5DC3_479A_823A_01D3F94B70B4

#include "yeti/core/parser_fn.hpp"
#include "yeti/core/parser_obj.hpp"

namespace yeti {

/**
 * @brief In yeti THE unit type is the regular-void for both values and errors.
 */
struct unit {
  static constexpr auto what() noexcept -> std::string_view { return {}; }
};

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
concept self_skip = std::same_as<P, skip_result_t<P>>;

template <typename P>
concept self_mute = std::same_as<P, mute_result_t<P>>;

template <typename... S>
using unit_unless_void =
    std::conditional_t<(std::is_void_v<S> && ...), void, unit>;

/**
 * @brief Recursive implementation of the parser concept.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
struct parser_impl : std::false_type {};

// If neither P or S know the stream type then don't check for unit.
template <typename P, typename S, typename T>
using mute_recur_help =
    parser_impl<P, S, T, unit_unless_void<S, static_type_of<P>>>;

// Propagate static type into recursive call
template <typename P, typename S, typename T>
using mute_recur = mute_recur_help<mute_result_t<P>, else_static<S, P>, T>;

// If neither P or S know the stream type then don't check for unit.
template <typename P, typename S, typename E>
using skip_recur_help =
    parser_impl<P, S, unit_unless_void<S, static_type_of<P>>, E>;

// Propagate static type into recursive call
template <typename P, typename S, typename E>
using skip_recur = skip_recur_help<skip_result_t<P>, else_static<S, P>, E>;

// Terminal case mute and skip OK
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_mute<P> && self_skip<P>
struct parser_impl<P, S, T, E> : std::true_type {};

// Skip OK check mute
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_skip<P>
struct parser_impl<P, S, T, E> : mute_recur<P, S, T> {};

// Mute OK check skip
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E> && self_mute<P>
struct parser_impl<P, S, T, E> : skip_recur<P, S, E> {};

// Neither OK, recurse both
template <typename P, typename S, typename T, typename E>
  requires parser_obj<P, S, T, E>
struct parser_impl<P, S, T, E>
    : std::conjunction<mute_recur<P, S, T>, skip_recur<P, S, E>> {};

/**
 * @brief The full canonical definition of a parser.
 *
 * This implementation fully subsumes and additional checks in
 * the `parser` concept, they are there to provide better error
 * messages.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept canonical_parser = parser_impl<P, S, T, E>::value;

// ===  === //
// ===  === //
// ===  === //

template <typename P, typename S = void, typename E = void>
concept core_parser_skip_help =
    parser_obj<P, S, unit_unless_void<S, static_type_of<P>>, E>;

template <typename P, typename S = void, typename T = void>
concept core_parser_mute_help =
    parser_obj<P, S, T, unit_unless_void<S, static_type_of<P>>>;

/**
 * @brief Yeti's defining concept, the parser.
 *
 * @tparam P The parser type.
 * @tparam S The range type over which the parser operates.
 * @tparam T The value type which the parser produces.
 * @tparam E The error type which the parser produces.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept parser =
    parser_obj<P, S, T, E>                                           //
    && core_parser_skip_help<skip_result_t<P>, else_static<S, P>, E> //
    && core_parser_mute_help<mute_result_t<P>, else_static<S, P>, T> //
    && canonical_parser<P, S, T, E>;                                 //

} // namespace impl::parser_concept

using impl::parser_concept::parser;

} // namespace yeti

#endif /* B87A46A7_5DC3_479A_823A_01D3F94B70B4 */
