#ifndef C718619C_BF1C_4319_8528_51F560F23EE1
#define C718619C_BF1C_4319_8528_51F560F23EE1

#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/**
 * @brief The yeti parser combinator library.
 *
 * The yeti library is a parser combinator library that is designed to be
 * simple, efficient and leverage  C++23 features. With the goal of
 * simplifying the implementation, yeti makes most of its types aggregates
 * and prefers inheritance over composition (even for standard library
 * types). This exposes some internals but saves a lot of boilerplate code.
 *
 * Yeti is constexpr
 *
 * Yeti tries to have descriptive error messages.
 *
 * Yeti has first class support for
 * - `skip`
 * - `ignore`/`mute`/`silence`/`discard`/
 * - `drop`
 *
 * Yeti is not currently generic over the input range, it only supports
 * `string_view`.
 *
 * Yeti provides the following parsers:
 *
 * - `noop` parser that does nothing and always succeeds.
 * - `fail` parser that does nothing and always fails.
 * - `any` parser that matches any char.
 * - `eof` parser that matches the end of the input.
 *
 * TODO: just, desc
 *
 *
 */

namespace yeti {

/**
 * @brief Ignore the impl namespace.
 */
namespace impl {

template <typename T, template <typename...> typename Primary>
struct specialization_impl : std::false_type {};

template <template <typename...> typename Primary, typename... Args>
struct specialization_impl<Primary<Args...>, Primary> : std::true_type {};

} // namespace impl

/**
 * @brief Test if a type is a specialization of a template.
 */
template <typename T, template <typename...> typename Primary>
concept specialization_of = impl::specialization_impl<T, Primary>::value;

/**
 * @brief Test if a type is any of the given types.
 */
template <typename T, typename... Args>
concept either = (std::same_as<T, Args> || ...);

/**
 * @brief Like std::moveable but without requiring assignment.
 */
template <typename T>
concept move_cons_object = std::is_object_v<T> && std::move_constructible<T>;

// /**
//  * @brief Test if a type can be derived from.
//  */
// template <typename T>
// concept derivable = std::is_class_v<T> && !std::is_final_v<T>;

/**
 * @brief A yeti error is produced on-demand via the `what` method.
 */
template <typename E>
concept error = move_cons_object<E> && requires (E e) {
  { e.what() } -> either<std::string, std::string_view>;
};

/**
 * @brief In yeti the unit type is the regular-void for both values and errors.
 */
struct unit {
  constexpr auto what() const noexcept -> std::string_view { return {}; }
};

/**
 * @brief A unit type that signals a value that cannot be produced.
 */
struct never {
  [[noreturn]] static constexpr auto what() -> std::string_view {
    std::unreachable();
  }
};

/**
 * @brief The result of invoking a yeti parser.
 *
 * A parser can succeed or fail, regardless it returns the unconsumed input.
 *
 * TIP: try destructuring the result type to get the value and the rest.
 *
 * @tparam T The type of the result of the parse.
 * @tparam E The type of the error of the parse.
 * @tparam R The type of the range that the parser consumes.
 */
template <std::ranges::range R, move_cons_object T, move_cons_object E>
struct result {
  [[no_unique_address]] std::expected<T, E> value; ///< The result of the parse.
  [[no_unique_address]] R rest;                    ///< Unconsumed input.
};

// TODO: do we need to terminate the recursion?

namespace impl::parser {

/**
 * In prose, a parser:
 *
 * 1. Is a function that can be invoked to a parse result<R, T, E>.
 * 2. It must have skip and mute methods.
 * 3. The invocation, skip and mute methods must be valid for all
 *    value categories.
 * 4. The invocations in (3) with the different value categories must
 *    return the same type independent of the value category.
 * 5. Is a move constructible object type.
 *
 * These first constraints are _core_ constraints.
 *
 * --
 *
 * 6. The result of .skip() is a parser -> result<R, unit, E>
 * 7. The result of .mute() is a parser -> result<R, T, unit>
 *
 */

// === Consistant invocation === //

template <typename F, typename G, typename... Args>
concept similar_invocable = std::invocable<F, Args...>               //
                            && std::invocable<G, Args...>            //
                            && std::same_as<                         //
                                   std::invoke_result_t<F, Args...>, //
                                   std::invoke_result_t<G, Args...>  //
                                   >;                                //

/**
 * @brief A helper for inspecting the result of a parser.
 */
template <typename, typename>
struct inspect_impl : std::false_type {};

template <typename R, typename T, typename E>
struct inspect_impl<R, result<R, T, E>> : std::true_type {
  using value_type = T;
  using error_type = E;
  using range_type = R;
};

template <typename P, typename R>
  requires std::invocable<P, R>
using inspect_result = inspect_impl<R, std::invoke_result_t<P, R>>;

template <typename P, typename R>
concept invokes_returns_result =
    std::invocable<P, R> && inspect_result<P, R>::value;

/**
 * @brief Check that `P(R) -> result<R, _, _>` for all invocations.
 *
 * All invocations must return the same result type.
 */
template <typename P, typename R>
concept core_parser_invocable = std::invocable<P, R>                    //
                                && invokes_returns_result<P, R>         //
                                && similar_invocable<P, P &, R>         //
                                && similar_invocable<P, P const &, R>   //
                                && similar_invocable<P, P &&, R>        //
                                && similar_invocable<P, P const &&, R>; //

// === Consistant skip === //

template <typename P>
concept skippable = requires (P val) { static_cast<P &&>(val).skip(); };

template <skippable P>
using skip_result_t = decltype(std::declval<P>().skip());

template <typename P, typename Q>
concept similar_skippable =
    skippable<P>                                         //
    && skippable<Q>                                      //
    && std::same_as<skip_result_t<P>, skip_result_t<Q>>; //

/**
 * @brief Check that `P.skip() -> _` is valid for all invocations.
 *
 * All the invocations must return the same type.
 */
template <typename P>
concept core_parser_skippable = skippable<P>                         //
                                && similar_skippable<P, P &>         //
                                && similar_skippable<P, P const &>   //
                                && similar_skippable<P, P &&>        //
                                && similar_skippable<P, P const &&>; //

// === Consistant mute === //

template <typename P>
concept muteable = requires (P val) { static_cast<P &&>(val).mute(); };

template <muteable P>
using mute_result_t = decltype(std::declval<P>().mute());

template <typename P, typename Q>
concept similar_muteable =
    muteable<P>                                          //
    && muteable<Q>                                       //
    && std::same_as<mute_result_t<P>, mute_result_t<Q>>; //

/**
 * @brief Check that `P.mute() -> _` is valid for all invocations.
 *
 * All the invocations must return the same type.
 */
template <typename P>
concept core_parser_muteable = muteable<P>                         //
                               && similar_muteable<P, P &>         //
                               && similar_muteable<P, P const &>   //
                               && similar_muteable<P, P &&>        //
                               && similar_muteable<P, P const &&>; //

// === Core parser def === //

/**
 * @brief Check 1-5 of the parser concept.
 *
 * When R is void then the check is for a generic
 * parser, i.e. the range type is not known yet.
 */
template <typename P, typename R = void>
concept unconstrained_core_parser =
    move_cons_object<P>                                        //
    && core_parser_skippable<P>                                //
    && core_parser_muteable<P>                                 //
    && (std::same_as<R, void> || core_parser_invocable<P, R>); //

template <typename P, typename R>
struct parse_result {
  using value_type = void;
  using error_type = void;
};

template <typename P, typename R>
  requires std::invocable<P, R>
struct parse_result<P, R> : inspect_result<P, R> {};

template <typename P, typename R = void>
  requires unconstrained_core_parser<P, R>
using parse_value_t = typename parse_result<P, R>::value_type;

template <typename P, typename R = void>
  requires unconstrained_core_parser<P, R>
using parse_error_t = typename parse_result<P, R>::error_type;

/**
 * @brief Check that `P(R) -> result<R, T, E>
 *
 * Specify any of R, T, E as `void` to ignore.
 *
 * This composite concept enforces
 */
template <typename P, typename R = void, typename T = void, typename E = void>
concept core_parser = unconstrained_core_parser<P, R>          //
                      && either<T, void, parse_value_t<P, R>>  //
                      && either<E, void, parse_error_t<P, R>>; //

// === Extend to recursive ===

/**
 * @brief The full canonical definition of a parser.
 *
 * This implementation fully subsumes and additional checks in
 * the `parser` concept, they are there to provide better error
 * messages.
 */
template <typename P, typename R = void, typename T = void, typename E = void>
struct parser_impl : std::false_type {};

template <typename P>
concept self_skip = std::same_as<P, skip_result_t<P>>;

template <typename P>
concept self_mute = std::same_as<P, mute_result_t<P>>;

template <typename P, typename R, typename T>
using mute_recur = parser_impl<mute_result_t<P>, R, T, unit>;

template <typename P, typename R, typename E>
using skip_recur = parser_impl<skip_result_t<P>, R, unit, E>;

// Terminal case mute and skip OK
template <typename P, typename R, typename T, typename E>
  requires core_parser<P, R, T, E> && self_mute<P> && self_skip<P>
struct parser_impl<P, R, T, E> : std::true_type {};

// Skip OK check mute
template <typename P, typename R, typename T, typename E>
  requires core_parser<P, R, T, E> && self_skip<P>
struct parser_impl<P, R, T, E> : mute_recur<P, R, T> {};

// Mute OK check skip
template <typename P, typename R, typename T, typename E>
  requires core_parser<P, R, T, E> && self_mute<P>
struct parser_impl<P, R, T, E> : skip_recur<P, R, E> {};

// Neither OK, recurse both
template <typename P, typename R, typename T, typename E>
  requires core_parser<P, R, T, E>
struct parser_impl<P, R, T, E>
    : std::conjunction<mute_recur<P, R, T>, skip_recur<P, R, E>> {};

template <typename P, typename R = void, typename T = void, typename E = void>
concept parser = core_parser<P, R, T, E>                      //
                 && core_parser<skip_result_t<P>, R, unit, E> //
                 && core_parser<mute_result_t<P>, R, T, unit> //
                 && parser_impl<P, R, T, E>::value;           //

} // namespace impl::parser

// /**
//  * @brief Fetch the result of invoking a parser.
//  *
//  * This is deliberately minimally constrained and does not
//  * imply that `P` is a parser.
//  */
// template <typename P, typename I>
//   requires impl::parser_over<P, I>
// using parse_result_t = std::invoke_result_t<P, I>;

// /**
//  * @brief Fetch the value-type of a parser.
//  */
// template <typename P, typename I>
//   requires impl::parser_over<P, I>
// using parse_value_t = parse_result_t<P>::value_type;

// /**
//  * @brief Fetch the error type of a parser.
//  */
// template <typename P, typename I>
//   requires impl::parser_over<P, I>
// using parse_error_t = parse_result_t<P>::error_type;

/**
 * @brief The core concept underpinning yeti.
 *
 * @tparam P
 * @tparam T
 * @tparam E
 * @tparam I
 */
// template <typename P, typename I, typename T = void, typename E = void>
// concept parser =
//     //
//     parser_over<P, I>
//     //

//     // TODO: add skip_customized() and ignore_customized()
//     && specialization_of<parse_result_t<P>, result>;

// /**
//  * @brief Test if a parser produces a specific type/error.
//  *
//  * If `T` or `E` are specified as `void` they are ignored.
//  */

// ================== Combinator ==================

// template <parser P>
// struct basic_parser_combinator;

// template <typename P>
// concept parser_combinator =
//     parser<P> && specialization_of<P, basic_parser_combinator>;

// namespace impl::map {

// template <typename P, typename F>
// concept mappable = parser<P> && std::invocable<F, parse_value_t<P>>;

/**
 * @brief
 *
 * We cannot really know if P is mappable until the call which will determine
 * the value category and
 */
// template <parser P, typename F>
// struct parser {

//   [[no_unique_address]] P parser;
//   [[no_unique_address]] F fun;

//   using value_type = parse_value_t<P>;
//   using error_type = parse_error_t<P>;

//   template <typename Self>
//   constexpr auto operator()(this Self &&self, std::string_view sv) -> {

//     auto x = std::invoke(std::forward<Self>(self).parser, sv);

//     if (x) {
//       return result->map(fun);
//     }

//     return result.error();
//   }
// };

// } // namespace impl::map

// template <parser P>
// struct basic_parser_combinator {

//   [[no_unique_address]] P parser;

//   using value_type = parse_value_t<P>;
//   using error_type = parse_error_t<P>;

//   template <typename Self>
//   constexpr auto
//   operator()(this Self &&self, std::string_view sv) -> parse_result_t<P> {
//     return std::invoke(std::forward<Self>(self).parser, sv);
//   }

//   // template <typename Self, typename F>
//   // constexpr auto map(this Self &&self, F &&f) -> parser_combinator auto {
//   //   return basic_parser_combinator{
//   //       mapped{std::forward<Self>(self), std::forward<F>(f)},
//   //   };
//   // }
// };

} // namespace yeti

#endif /* C718619C_BF1C_4319_8528_51F560F23EE1 */
