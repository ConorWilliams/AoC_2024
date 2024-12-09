#ifndef CA0BCFB1_B572_4DB2_8DC2_C11079253A5A
#define CA0BCFB1_B572_4DB2_8DC2_C11079253A5A

#include <concepts>
#include <expected>
#include <type_traits>

#include "yeti/core/generics.hpp"
#include "yeti/core/result.hpp"
#include "yeti/core/typed.hpp"

namespace yeti {

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
struct inspect_impl<S, result<strip<S>, T, E>> : std::true_type {
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

template <typename P, typename S>
concept unconstrained_parser_fn_impl =
    invoke_returns_result<P, S>             //
    && similar_invocable<P, P &, S>         //
    && similar_invocable<P, P const &, S>   //
    && similar_invocable<P, P &&, S>        //
    && similar_invocable<P, P const &&, S>; //

template <typename P, typename S = void>
concept unconstrained_parser_fn_help =
    pure_void<S> || unconstrained_parser_fn_impl<P, S>;

/**
 * @brief Check that `P(S) -> result<strip<S>, _, _>` for all invocations.
 *
 * This passes if `S` is `void`.
 */
template <typename P, typename S = void>
concept unconstrained_parser_fn = unconstrained_parser_fn_help<strip<P>, S>; //

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
using raw_value_t = detail::parse_result<P, S>::value_type;

template <typename P, typename S = void>
using raw_error_t = detail::parse_result<P, S>::error_type;

/**
 * @brief Verify that `P(S)` returns a `result<_, T, _>`.
 *
 * If `S` is `void` then this will fail unless `T` is `void`.
 *
 * For better error messages this is a named concept
 */
template <typename P, typename S, typename T = void>
concept value_matches_request = either<T, void, raw_value_t<P, S>>;

/**
 * @brief Verify that `P(S)` returns a `result<_, _, E>`.
 *
 * If `S` is `void` then this will fail unless `E` is `void`.
 *
 * For better error messages this is a named concept
 */
template <typename P, typename S, typename E = void>
concept error_matches_request = either<E, void, raw_error_t<P, S>>;

/**
 * @brief Check that `P(S) -> result<S, T, E>`.
 *
 * Specify any of `S`, `T`, `E` as `void` to ignore.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept untyped_parser_fn = unconstrained_parser_fn<P, S>      //
                            && value_matches_request<P, S, T>  //
                            && error_matches_request<P, S, E>; //

// ===  === //
// ===  === //
// ===  === //

template <typename P, typename S = void>
concept type_matches =
    either<void, S, type_of<P>> || std::common_with<S, type_of<P>>;

template <typename P, typename S1, typename S2>
concept same_invoke_result =
    std::same_as<std::invoke_result_t<P, S1>, std::invoke_result_t<P, S2>>;

/**
 * @brief
 *
 * Assuming `S1 <- S` and `S2 <- type_of<P>`
 *
 * Match (S1, S2):
 *    (void, void) => untyped_parser_fn<P, void, T, E>  // Fails if T/E not void
 *    (void, S2)    => untyped_parser_fn<P, S2, T, E>
 *    (S1, void)    => untyped_parser_fn<P, S1, T, E>
 *    (S1, S2)       => UPF<P, S1, T, E> && UPF<P, S2, T, E> && same-result
 *
 * TIP: You can read: `!A || B` as `A => B` aka `A` implies `B`.
 */
template <typename P, typename S1, typename S2, typename T, typename E>
concept parser_fn_help =                                    //
    (!pure_void<S1, S2> || pure_void<T, E>)                 //
    &&(!not_void<S1> || untyped_parser_fn<P, S1, T, E>)     //
    &&(!not_void<S2> || untyped_parser_fn<P, S2, T, E>)     //
    &&(!not_void<S1, S2> || same_invoke_result<P, S1, S2>); //

/**
 * @brief The lowest level of the parser concept.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept parser_fn =                            //
    std::copy_constructible<P>                 //
    && type_matches<P, S>                      //
    && parser_fn_help<P, type_of<P>, S, T, E>; //

} // namespace impl::parser_fn_concept

using impl::parser_fn_concept::parser_fn;

/**
 * @brief Fetch the value type of a parser.
 *
 * This returns `void` if `S` is `void` and the parser is untyped.
 */
template <typename P, typename S = void>
  requires parser_fn<P, S>
using parse_value_t =
    impl::parser_fn_concept::raw_value_t<P, impl::else_static<S, P>>;

/**
 * @brief Fetch the error type of a parser.
 *
 * This returns `void` if `S` is `void` and the parser is untyped.
 */
template <typename P, typename S = void>
  requires parser_fn<P, S>
using parse_error_t =
    impl::parser_fn_concept::raw_error_t<P, impl::else_static<S, P>>;

} // namespace yeti

#endif /* CA0BCFB1_B572_4DB2_8DC2_C11079253A5A */
