

#include <concepts>
#include <expected>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

#include "yeti/core/atom.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/lift.hpp"
#include "yeti/core/parser.hpp"
#include "yeti/core/parser_fn.hpp"

struct empty {};

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

using namespace yeti;

struct pure {
  static constexpr auto
  operator()(std::string_view sv) -> result<std::string_view, char, int> {
    if (sv.starts_with("h")) {
      return {sv.substr(1), 'h'};
    }

    return {sv, std::unexpected(1)};
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

// rebind<decltype(pp), std::string_view const &>;

using T = std::invoke_result_t<const yeti::impl::parser_lift::skip_parser<
                                   yeti::impl::parser_lift::lifted<X>>,
                               const std::basic_string_view<char> &>;

static_assert(std::same_as<T, result<std::string_view, unit, int>>);

static_assert(parser_fn<decltype(p), std::string_view const &>);
constexpr auto y = p(sv);
static_assert(y.expected.value() == unit{});

struct half { //
  static auto
  operator()(std::string_view sv) -> result<std::string_view, char, int> {
    if (sv.starts_with("h")) {
      return {sv.substr(1), 'h'};
    }

    return {sv, std::unexpected(1)};
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

  static auto operator()(int) -> result<int, int, int>;
};

static_assert(parser_fn<typed_2, void>);
static_assert(parser_fn<typed_2, int>);
static_assert(!parser_fn<typed_2, double>);

// 3. Can be invoked with an `R` to produce a `result<R, T, E>`.

using SV = std::string_view;

template <class R>
using G = decltype([](std::string_view) -> R {
  std::unreachable();
});

static_assert(parser_fn<G<result<SV, int, int>>>);
static_assert(parser_fn<G<result<SV, int, int>>, SV>);
static_assert(parser_fn<G<result<SV, int, int>>, SV, int>);
static_assert(parser_fn<G<result<SV, int, int>>, SV, void, int>);
static_assert(parser_fn<G<result<SV, int, int>>, SV, int, int>);

static_assert(!parser_fn<G<result<int, int, int>>, SV>);
static_assert(!parser_fn<G<result<SV, int, int>>, int>);
static_assert(!parser_fn<G<int>, SV>);

static_assert(!parser_fn<G<result<SV, int, int>>, SV, long>);
static_assert(!parser_fn<G<result<SV, int, int>>, SV, void, long>);

static_assert(parser_fn<G<result<SV, int, int>> &, SV>);

// 4. Invocation must be valid for all value categories (VC).

struct mut_only {
  auto operator()(int) -> result<int, int, int>;
};

static_assert(!parser_fn<mut_only, int>);

// 5. Invocations in (3) must return the same type independent of the VC.

struct overloaded_ok {
  auto operator()(int) const -> result<int, int, int>;
  auto operator()(int) && -> result<int, int, int>;
};

static_assert(parser_fn<overloaded_ok, int>);

struct overloaded_bad {
  auto operator()(int) const -> result<int, int, int>;
  auto operator()(int) && -> result<int, long, int>;
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

struct never {};

template <typename E>
struct noop {
  using type = int;

  static auto operator()(int) -> result<int, unit, E>;

  static auto skip() -> noop;

  static auto mute() -> noop<unit>;
};

static_assert(parser<noop<never>>);

} // namespace

int main() { return 0; }