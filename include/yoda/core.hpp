#ifndef BFD65DCE_3DE3_4268_95B8_B951A870AF58
#define BFD65DCE_3DE3_4268_95B8_B951A870AF58

#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * @brief A small parser combinator library, inspired by Haskell's Yoda.
 */
namespace yoda {

// ====== Core defs

/**
 * @brief The result of invoking a parser.
 */
template <typename T> struct result : std::expected<T, std::string> {
  std::string_view rest;
};

namespace detail {

template <typename T> struct parser_impl : std::false_type {};

template <typename T> struct parser_impl<result<T>> : std::true_type {
  using type = T;
};

template <typename P>
concept is_result = parser_impl<P>::value;

} // namespace detail

/**
 * @brief Core concept for a parser.
 *
 * A parser is expected to be cheaply copyable.
 */
template <typename P>
concept parser =
    std::copy_constructible<P> && requires(P parser, std::string_view sv) {
      { std::invoke(parser, sv) } -> detail::is_result;
    };

/**
 * @brief Fetch the result of invoking a parser.
 */
template <parser P>
using parser_t =
    detail::parser_impl<std::invoke_result_t<P, std::string_view>>::type;

/**
 * @brief Test if a parser produces a specific type.
 */
template <typename P, typename T>
concept parser_of = parser<P> && std::same_as<T, parser_t<P>>;

/**
 * @brief Attempt to parse a string with a parser.
 *
 * This throws an exception if the parse fails.
 */
template <typename P>
constexpr auto parse(P p, std::string_view sv) -> parser_t<P> {

  auto r = p(sv);

  if (r) {
    return std::move(r).value();
  }

  constexpr std::string_view fmt = "Parser error:\n\t{}\nRemainder:\n\t{}";

  throw std::runtime_error(std::format(fmt, r.error(), r.rest));
}

} // namespace yoda

#endif /* BFD65DCE_3DE3_4268_95B8_B951A870AF58 */
