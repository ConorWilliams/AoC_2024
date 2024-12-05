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

// ===================== Extend to recursive ===================== //

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
