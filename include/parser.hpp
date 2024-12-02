#ifndef CE2053D2_90CB_4935_B5A1_4A928C8AB29F
#define CE2053D2_90CB_4935_B5A1_4A928C8AB29F

#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <version>

/**
 * @brief A small parser combinator library, inspired by Haskell's Yoda.
 */
namespace yoda {

// ====== Core defs

struct mono {};

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

template <typename P>
concept parser =
    std::copy_constructible<P> && requires(P parser, std::string_view sv) {
      { std::invoke(parser, sv) } -> detail::is_result;
    };

template <parser P>
using parser_t =
    detail::parser_impl<std::invoke_result_t<P, std::string_view>>::type;

template <typename P, typename T>
concept parser_of = parser<P> && std::same_as<T, parser_t<P>>;

// ====== Util

template <typename P>
constexpr auto parse(P p, std::string_view sv) -> parser_t<P> {

  auto r = p(sv);

  if (r) {
    return std::move(r).value();
  }

  std::string msg =
      std::format("Parser error:\n\t{}\nRemainder:\n\t{}", r.error(), r.rest);

  throw std::runtime_error(std::move(msg));
}

template <std::convertible_to<std::string> T>
constexpr auto unexpected(T &&message) -> std::unexpected<std::string> {
  return std::unexpected(std::string{std::forward<T>(message)});
}

// ======

constexpr auto any = [](std::string_view sv) -> result<char> {
  if (sv.empty()) {
    return {unexpected("Expected any character but got EoF"), {}};
  }
  return {sv[0], sv.substr(1)};
};

static_assert(parser_of<decltype(any), char>);

constexpr auto eof = [](std::string_view sv) -> result<mono> {
  if (sv.empty()) {
    return {mono{}, {}};
  }
  return {unexpected(std::format("Expected EoF but got '{}'", sv)), sv};
};

static_assert(parser_of<decltype(eof), mono>);

// ====== Combinators

constexpr auto alt = []<parser P, parser Q>(P p, Q q) {
  //
  using RQ = parser_t<Q>;
  using RP = parser_t<P>;

  constexpr bool same = std::is_same_v<RP, RQ>;

  using R = result<std::conditional_t<same, RP, std::variant<RP, RQ>>>;

  return [p = std::move(p), q = std::move(q)](std::string_view sv) -> R {
    //
    auto lhs = p(sv);

    if (lhs) {
      return {std::move(lhs).value(), lhs.rest};
    }

    auto rhs = q(sv);

    if (rhs) {
      return {std::move(rhs).value(), rhs.rest};
    }

    auto msg = std::format("Both parsers failed with:\n\t{}\n\t{}", lhs.error(),
                           rhs.error());

    return {unexpected(std::move(msg)), sv};
  };
};

} // namespace yoda

#endif /* CE2053D2_90CB_4935_B5A1_4A928C8AB29F */
