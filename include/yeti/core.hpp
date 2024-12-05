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
 * TODO: move the range::range constraint to something weaker.
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
  static constexpr auto what() noexcept -> std::string_view { return {}; }
};

/**
 * @brief A unit type that signals a value that cannot be produced.
 */
struct never {

  never() = delete;

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
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

/*



*/

/**
 * @brief Implementation of the `parser` concept.
 *
 * Before reading this code it is recommended to read the
 * documentation/comment in the `parser` concept which
 * explains the constraints.
 */
namespace impl::parser_concept {

// ===================== Consistant invocation ===================== //

// Consider this type of parser:

/**
 * @brief A combinator that skips the result of a parser.
 */
template <parse_fn F>
stuct skipped {

  F fn;

  template <typename Self, typename S>
  auto operator()(this Self &&self, S &&stream) {

    auto [val, rest] =
        std::invoke(std::forward<Self>(self).fn, std::forward<S>(stream));

    if (val) {
      return result{unit, std::move(rest)};
    }

    return result{val.error(), std::move(rest)};
  }

  //
  // auto skip(this Self && self)->...;
}

template <parse_fn F>
stuct from_fn_impl {

  F fn;

  auto operator()(...) /* Perfect forward f(...) */;
}

// ===================== Consistant skip ===================== //

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

// ===================== Consistant mute ===================== //

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

// ===================== Core parser def ===================== //

/**
 * @brief Check 1-4 of the parser concept.
 *
 * When R is void then the check is for a generic
 * parser, i.e. the range type is not known yet.
 */
template <typename P, typename R = void>
concept unconstrained_core_parser =
    core_parser_skippable<P>                                   //
    && core_parser_muteable<P>                                 //
    && (std::same_as<R, void> || core_parser_invocable<P, R>); //

// ===================== Extend to recursive ===================== //

/**
 * Concepts cannot be recursive, so this must be specified the old
 * fashioned way. This is then partially duplicated in concept-land
 * for be better error messages.
 */

template <typename P>
concept self_skip = std::same_as<P, skip_result_t<P>>;

template <typename P>
concept self_mute = std::same_as<P, mute_result_t<P>>;

template <typename... R>
using unit_unless_void =
    std::conditional_t<(std::is_void_v<R> && ...), void, unit>;

/**
 * @brief Recursive implementation of the parser concept.
 */
template <typename P, typename R = void, typename T = void, typename E = void>
struct parser_impl : std::false_type {};

// If neither P or R know the stream type then don't check for unit.
template <typename P, typename R, typename T>
using mute_recur_help =
    parser_impl<P, R, T, unit_unless_void<R, static_type_of<P>>>;

// Propagate static type into recursive call
template <typename P, typename R, typename T>
using mute_recur = mute_recur_help<mute_result_t<P>, else_static<R, P>, T>;

// If neither P or R know the stream type then don't check for unit.
template <typename P, typename R, typename E>
using skip_recur_help =
    parser_impl<P, R, unit_unless_void<R, static_type_of<P>>, E>;

// Propagate static type into recursive call
template <typename P, typename R, typename E>
using skip_recur = skip_recur_help<skip_result_t<P>, else_static<R, P>, E>;

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

/**
 * @brief The full canonical definition of a parser.
 *
 * This implementation fully subsumes and additional checks in
 * the `parser` concept, they are there to provide better error
 * messages.
 */
template <typename P, typename R = void, typename T = void, typename E = void>
concept canonical_parser = parser_impl<P, R, T, E>::value;

// === Small helpers === //

template <typename P, typename R = void, typename E = void>
concept core_parser_skip_help =
    core_parser<P, R, unit_unless_void<R, static_type_of<P>>, E>;

template <typename P, typename R = void, typename T = void>
concept core_parser_mute_help =
    core_parser<P, R, T, unit_unless_void<R, static_type_of<P>>>;

/**
 * @brief Yeti's defining concept, the parser.
 *
 * In prose, a parser:
 *
 * 1. Can be invoked with an `R` to produce a `result<R, T, E>`.
 * 2. Has `.skip()` and `.mute()` methods.
 * 3. Calls in (1) and (2) must be valid for all value categories (VC).
 * 4. The calls in (3) must return the same type independent of the VC.
 * 5. Is a move constructible object type.
 * 6. If `P` defines a `::type` then R matches it.
 *
 *
 * _. The result of `.skip()` is a `parser<R unit, E>`.
 * _. The result of `.mute()` is a `parser<R, T unit>`.
 *
 * Conditions 1-5 are termed _core_ constraints (because they do
 * not require the recursive checks).
 *
 * The template parameters `R`, `T`, `E` can be `void` to ignore.
 *
 * @tparam P The parser type.
 * @tparam R The range type over which the parser operates.
 * @tparam T The value type which the parser produces.
 * @tparam E The error type which the parser produces.
 */
template <typename P, typename R = void, typename T = void, typename E = void>
concept parser =
    core_parser<P, R, T, E>                                          //
    && core_parser_skip_help<skip_result_t<P>, else_static<R, P>, E> //
    && core_parser_mute_help<mute_result_t<P>, else_static<R, P>, T> //
    && canonical_parser<P, R, T, E>;                                 //

} // namespace impl::parser_concept

using impl::parser_concept::parser;

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