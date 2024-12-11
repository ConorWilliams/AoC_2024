

#include <concepts>
#include <expected>
#include <iostream>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "yeti/core.hpp"
// #include "yeti/generic/range.hpp"
#include "yeti/core/flat_variant.hpp"
#include "yeti/generic/trivial.hpp"

using SV = std::string_view;

using namespace std::literals;
using namespace yeti;

flat_variant<int> x;

// =====
// =====
// =====
// =====
// =====
// =====
// =====
// =====
// =====

struct errr {
  static auto what() -> std::string_view { return "err"; }
};

struct to_lift_never1 {
  static constexpr auto operator()(SV x) -> result<SV, never, errr> {
    return {x, std::unexpected(errr{})};
  }
};

constexpr auto l1 = lift(to_lift_never1{});

static_assert(parser<decltype(l1), SV>);

static_assert(parser_fn<to_lift_never1, SV>);

// =====
// =====
// =====
// =====
// =====
// =====
// =====
// =====
// =====

struct not_parser {
  static constexpr auto skip() -> not_parser;
  static constexpr auto mute() -> not_parser;

  template <typename T>
  static constexpr auto operator()(T) -> result<T, int, never>;
};

static_assert(!parser<not_parser, int, int, never>);

struct empty {};

static_assert(std::copy_constructible<int const>);

constexpr auto k = fail.drop();

static_assert(unit{} == unit{});

// import string view literal

static_assert(!eos("hhh"sv));

// constexpr int i = k;

struct empty_empty {
  [[no_unique_address]] empty k{};
};

static_assert(std::is_empty_v<empty_empty>);

namespace {

static_assert(std::common_with<int, float &>);

static_assert(std::common_with<std::string_view &, std::string>);

template <typename T>
using sub = decltype(std::ranges::subrange(std::declval<T>()));

// static_assert(std::same_as<sub<std::string_view>, std::string_view>);

struct pure {
  static constexpr auto
  operator()(std::string_view sv) -> result<std::string_view, char, unit> {
    if (sv.starts_with("h")) {
      return {sv.substr(1), 'h'};
    }

    return {sv, std::unexpected(unit{})};
  }
};

inline constexpr auto x = pure{};

using X = pure;

static_assert(parser_fn<X>);

impl::parser_lift::lifted<X> pp{};

static_assert(parser_fn<decltype(pp), std::string_view>);

// static_assert(std::destructible<>);

constexpr auto p = lift(x).skip().mute();
constexpr std::string_view sv{"hello"};
static_assert(
    specialization_of<rebind<decltype(x), std::string_view, unit>, result>);

static_assert(parser_fn<decltype(pp), std::string_view>);

static_assert(parser_fn<decltype(p), std::string_view const &>);
constexpr auto y = p(sv);
static_assert(y.expected.value() == unit{});

struct half { //
  static auto
  operator()(std::string_view sv) -> result<std::string_view, char, unit> {
    if (sv.starts_with("h")) {
      return {sv.substr(1), 'h'};
    }

    return {sv, std::unexpected(unit{})};
  }

  constexpr auto skip() -> todo { return {}; }
};

static_assert(parser_fn<half, std::string_view>);

constexpr auto _ = half{}.skip();

constexpr auto s = lift(half{}).skip().mute();

template <typename T>
struct CTAD {
  T x;
};

constexpr int j = 0;

// int i = std::type_identity<decltype(CTAD{j})>{};

// static_assert(
//     std::same_as<int, parse_error_t<decltype(p), std::string_view const
//     &>>);

// // template <typename T = std::string>
// // constexpr auto test(T &&) -> int {

// //   static_assert(std::same_as<T, void>);

// //   return 0;
// // }

// constexpr auto x = test({});

// ================================================================ //

// 1. Is a move constructible type.

using F = decltype([] {});

static_assert(parser_fn<F>);
static_assert(parser_fn<F &>);
static_assert(!parser_fn<F, int>);

struct immovable {
  immovable() = default;
  immovable(immovable &&) = delete;
};

static_assert(!parser_fn<immovable>);

// 2. If `P` defines a `::type` then R matches it.

struct typed_1 {
  using type = int;
};

static_assert(!parser_fn<typed_1, void>);

struct typed_2 {
  //
  using type = int;

  static auto operator()(int) -> result<int, int, unit>;
};

static_assert(parser_fn<typed_2, void>);
static_assert(parser_fn<typed_2, int>);

static_assert(parser_fn<typed_2, void, int>);

static_assert(!parser_fn<typed_2, double>);

// 3. Can be invoked with an `R` to produce a `result<R, T, E>`.

template <class R>
using G = decltype([](std::string_view) -> R {
  std::unreachable();
});

static_assert(parser_fn<G<result<SV, int, unit>>>);
static_assert(parser_fn<G<result<SV, int, unit>>, SV>);
static_assert(parser_fn<G<result<SV, int, unit>>, SV, int>);
static_assert(parser_fn<G<result<SV, int, unit>>, SV, void, unit>);
static_assert(!parser_fn<G<result<SV, int, unit>>, void, void, unit>);
static_assert(parser_fn<G<result<SV, int, unit>>, SV, int, unit>);

static_assert(!parser_fn<G<result<int, int, unit>>, SV>);
static_assert(!parser_fn<G<result<SV, int, unit>>, int>);
static_assert(!parser_fn<G<int>, SV>);

static_assert(!parser_fn<G<result<SV, int, unit>>, SV, long>);
static_assert(!parser_fn<G<result<SV, int, unit>>, SV, void, long>);

static_assert(parser_fn<G<result<SV, int, unit>> &, SV>);

// 4. Invocation must be valid for all value categories (VC).

struct mut_only {
  auto operator()(int) -> result<int, int, unit>;
};

static_assert(!parser_fn<mut_only, int>);

// 5. Invocations in (3) must return the same type independent of the VC.

struct overloaded_ok {
  auto operator()(int) const -> result<int, int, unit>;
  auto operator()(int) && -> result<int, int, unit>;
};

static_assert(parser_fn<overloaded_ok, int>);

struct overloaded_bad {
  auto operator()(int) const -> result<int, int, unit>;
  auto operator()(int) && -> result<int, long, unit>;
};

static_assert(!parser_fn<overloaded_bad, int>);

// ================================================================ //

struct H {
  using type = int;

  static auto operator()(int) -> result<int, unit, unit>;
  static auto skip() -> H;
  static auto mute() -> H;
};

static_assert(parser<H>);

static_assert(parser<H, int>);

struct never {
  static auto what() -> std::string_view { return "never"; }
};

template <typename E>
struct noop {
  using type = int;

  static auto operator()(int) -> result<int, unit, E>;

  static auto skip() -> noop;

  static auto mute() -> noop<unit>;
};

static_assert(parser<noop<never>>);

} // namespace

struct TypeTeller {
  auto speak(this auto &&self) -> int {
    using SelfType = decltype(self);
    using UnrefSelfType = std::remove_reference_t<SelfType>;
    if constexpr (std::is_lvalue_reference_v<SelfType>) {
      if constexpr (std::is_const_v<UnrefSelfType>)
        std::cout << "const lvalue\n";
      else
        std::cout << "mutable lvalue\n";
    } else {
      if constexpr (std::is_const_v<UnrefSelfType>)
        std::cout << "const rvalue\n";
      else
        std::cout << "mutable rvalue\n";
    }

    return 0;
  }
};

struct noisy {
  noisy() { std::cout << "construct\n"; }
  noisy(noisy &&) { std::cout << "move\n"; }
  noisy(noisy const &) { std::cout << "copy\n"; }
  ~noisy() { std::cout << "destruct\n"; }
};

auto tell() {

  TypeTeller tell{};

  return std::move(tell).speak();
}

struct innet {
  noisy in;
};

auto cons() -> innet {
  noisy n{};
  return {{std::move(n)}};
}

int main() {

  tell();

  cons();

  return 0;
}