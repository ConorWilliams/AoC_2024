#ifndef CA0BCFB1_B572_4DB2_8DC2_C11079253A5A
#define CA0BCFB1_B572_4DB2_8DC2_C11079253A5A

#include <concepts>
#include <expected>
#include <string>
#include <string_view>
#include <type_traits>

#include "yeti/core/generics.hpp"
#include "yeti/core/typed.hpp"

namespace yeti {

/**
 * @brief A yeti error is produced on-demand via the `what` method.
 */
template <typename E>
concept error = std::movable<E> && requires (E e) {
  { e.what() } -> either<std::string, std::string_view>;
};

/**
 * @brief The result of invoking a yeti parser.
 *
 * A parser can succeed or fail, regardless it returns the unconsumed input.
 *
 * TIP: Try destructuring the result type to get the value and the rest.
 *
 * @tparam T The type of the result of the parse.
 * @tparam E The type of the error of the parse.
 * @tparam S The type of stream that the parser consumes.
 */
template <std::movable S, std::movable T, std::movable E>
struct result {
  [[no_unique_address]] S rest;                    ///< Unconsumed input.
  [[no_unique_address]] std::expected<T, E> value; ///< The result of the parse.
};

/**
 * @brief A customization point for defining the result of parsing a stream.
 */
template <typename S>
struct decay : std::decay<S> {};

template <typename S>
using decay_t = decay<S>::type;

// ===  === //
// ===  === //
// ===  === //

/**
 * @brief Implementation of the parser_fn concept.
 */
namespace impl::parser_fn_concept {

/**
 * @brief Require that `F(Args...) == G(Args...)`.
 */
template <typename F, typename G, typename... Args>
concept similar_invocable = std::invocable<F, Args...>               //
                            && std::invocable<G, Args...>            //
                            && std::same_as<                         //
                                   std::invoke_result_t<F, Args...>, //
                                   std::invoke_result_t<G, Args...>  //
                                   >;                                //

namespace detail {

template <typename, typename>
struct inspect_impl : std::false_type {};

template <typename S, typename T, typename E>
struct inspect_impl<S, result<std::decay_t<S>, T, E>> : std::true_type {
  using value_type = T;
  using error_type = E;
};

} // namespace detail

template <typename P, typename S>
  requires std::invocable<P, S>
using inspect_result = detail::inspect_impl<S, std::invoke_result_t<P, S>>;

template <typename P, typename S>
concept invoke_returns_result =
    std::invocable<P, S> && inspect_result<P, S>::value;

/**
 * @brief Check that `P(S) -> result<S, _, _>` for all invocations.
 *
 * All invocations must return the same result type.
 *
 * This concept is not tolerant of `S = void` and P should be unqualified.
 */
template <typename P, typename S>
concept unconstrained_parser_fn_impl =
    invoke_returns_result<P, S>             //
    && similar_invocable<P, P &, S>         //
    && similar_invocable<P, P const &, S>   //
    && similar_invocable<P, P &&, S>        //
    && similar_invocable<P, P const &&, S>; //

template <typename P, typename S = void>
concept unconstrained_parser_fn_help =
    std::same_as<S, void> || unconstrained_parser_fn_impl<P, S>;

template <typename P, typename S = void>
concept unconstrained_parser_fn =
    std::move_constructible<P>                                  //
    && unconstrained_parser_fn_help<std::remove_cvref_t<P>, S>; //

// ===  === //
// ===  === //
// ===  === //

/**
 * This is needed because inspect_result does not
 * short-circuiting when S is void.
 */
namespace detail {

template <typename P, typename S = void>
struct parse_result : inspect_result<P, S> {};

template <typename P>
struct parse_result<P, void> {
  using value_type = void;
  using error_type = void;
};

} // namespace detail

template <typename P, typename S = void>
using parse_value_t = detail::parse_result<P, S>::value_type;

template <typename P, typename S = void>
using parse_error_t = detail::parse_result<P, S>::error_type;

// For better error messages
template <typename P, typename S = void, typename T = void>
concept value_matches_request = either<T, void, parse_value_t<P, S>>;

template <typename P, typename S = void, typename E = void>
concept error_matches_request = either<E, void, parse_error_t<P, S>>;

/**
 * @brief Check that `P(S) -> result<S, T, E>
 *
 * Specify any of S, T, E as `void` to ignore.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept untyped_parser_fn = unconstrained_parser_fn<P, S>      //
                            && value_matches_request<P, S, T>  //
                            && error_matches_request<P, S, E>; //

// ===  === //
// ===  === //
// ===  === //

template <typename P, typename S = void>
concept static_type_matches =
    either<void, S, static_type_of<P>> || std::same_as<S, static_type_of<P>>;

/**
 * @brief The lowest level of the parser concept.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept parser_fn = std::move_constructible<P>                        //
                    && static_type_matches<P, S>                      //
                    && untyped_parser_fn<P, else_static<S, P>, T, E>; //

} // namespace impl::parser_fn_concept

using impl::parser_fn_concept::parser_fn;

/**
 * @brief Fetch the value type of a parser.
 *
 * This returns `void` if `S` is `void` and the parser is untyped.
 */
template <typename P, typename S = void>
  requires parser_fn<P, S>
using parse_value_t = impl::parser_fn_concept::parse_value_t<P, S>;

/**
 * @brief Fetch the error type of a parser.
 *
 * This returns `void` if `S` is `void` and the parser is untyped.
 */
template <typename P, typename S = void>
  requires parser_fn<P, S>
using parse_error_t = impl::parser_fn_concept::parse_error_t<P, S>;

namespace impl {

template <typename T>
inline constexpr bool False = false;

template <typename R, typename...>
struct rebind;

template <typename S, typename T, typename E, typename t, typename e>
struct rebind<result<S, T, E>, t, e> {
  using type = result<S, t, e>;
};

template <typename S, typename T, typename E, typename t>
struct rebind<result<S, T, E>, t, void> {
  using type = result<S, t, E>;
};

template <typename S, typename T, typename E, typename e>
struct rebind<result<S, T, E>, void, e> {
  using type = result<S, T, e>;
};

template <typename S, typename T, typename E>
struct rebind<result<S, T, E>, void, void> {
  using type = result<S, T, E>;
};

} // namespace impl

/**
 * @brief Rebind the result of a parser.
 *
 * I.e. If `P(S) -> result<'S, 'T, 'E>` then `rebind<P, S, T, E>` is
 * `P(S) -> result<`S, 'T if is_void<T> else T, 'E if is_void<E> else E>`.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
  requires parser_fn<P, S> && (typed<P> || !std::same_as<S, void>)
using rebind = impl::rebind<std::invoke_result_t<P, S>, T, E>::type;

} // namespace yeti

#endif /* CA0BCFB1_B572_4DB2_8DC2_C11079253A5A */
