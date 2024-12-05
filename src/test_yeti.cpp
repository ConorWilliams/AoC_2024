

#include "yeti/core/parser_fn.hpp"
#include <string_view>
#include <utility>

namespace {

using namespace yeti;

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

} // namespace

int main() { return 0; }