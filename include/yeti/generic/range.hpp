#ifndef BCC882F7_ED8D_4A23_B468_977755EA6D56
#define BCC882F7_ED8D_4A23_B468_977755EA6D56

#include <expected>
#include <format>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <concepts>

#include "yeti/core.hpp"
#include "yeti/core/flat_variant.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"
#include "yeti/generic/trivial.hpp"

namespace yeti {

template <typename T>
concept recombinant_range = storable<T> && std::ranges::range<T> &&
                            std::constructible_from<strip<T>,
                                                    std::ranges::iterator_t<T>,
                                                    std::ranges::sentinel_t<T>>;
template <typename T>
concept recombinant_input_range = std::ranges::input_range<T> && recombinant_range<T>;

template <typename T>
concept recombinant_forward_range = std::ranges::forward_range<T> && recombinant_range<T>;

namespace impl::any_impl {

struct eos {
  [[nodiscard]] static constexpr auto what() noexcept -> std::string_view {
    return "Satisfy expects a token but got end of stream";
  }
};

/**
 * @brief The result of accessing `t.fn` when `t` is a `T`.
 */
template <typename T>
using forward_fn_t = decltype(std::declval<T>().fn);

template <typename>
struct expected_invocable_r_impl : std::false_type {};

template <error E>
struct expected_invocable_r_impl<std::expected<unit, E>> : std::true_type {};

template <typename R>
concept expected_invocable_r = expected_invocable_r_impl<R>::value;

template <typename F, typename I>
concept expected_invocable_help = std::indirectly_unary_invocable<F, I> &&
                                  expected_invocable_r<std::indirect_result_t<F, I>>;

template <typename Self, typename R>
concept expected_invocable =
    expected_invocable_help<forward_fn_t<Self>, std::ranges::iterator_t<R>>;

/* F must be copy constructible such that we can satisfy parser_fn */
template <std::copy_constructible F>
struct satisfy final {

  static_assert(std::same_as<F, strip<F>>);

  [[no_unique_address]] F fn;

  template <typename Self, typename S>
    requires recombinant_input_range<S> && expected_invocable<Self, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> specialization_of<result> auto {

    using I = std::ranges::iterator_t<S>;
    using V = std::iter_value_t<I>;
    using R = std::indirect_result_t<forward_fn_t<Self>, I>;
    using E = R::error_type;

    using Err = flat_variant<eos, E>;
    using Res = resulting_t<S, V, Err>;
    using Exp = Res::expected_type;

    auto beg = std::ranges::begin(stream);
    auto end = std::ranges::end(stream);

    if constexpr (requires { bool(stream.empty()); }) {
      if (stream.empty()) {
        return Res{YETI_FWD(stream), Exp{std::unexpect, eos{}}};
      }
    } else if constexpr (requires { std::ranges::size(stream) == 0; }) {
      if (std::ranges::size(stream) == 0) {
        return Res{YETI_FWD(stream), Exp{std::unexpect, eos{}}};
      }
    } else {
      if (beg == end) {
        return Res{YETI_FWD(stream), Exp{std::unexpect, eos{}}};
      }
    }

    V tok = *beg;

    std::expected pred = std::invoke(YETI_FWD(self).fn, tok);

    if (pred) {
      return Res{
          {std::next(std::move(beg)), std::move(end)},
          {std::move(tok)},
      };
    }

    return Res{
        YETI_FWD(stream),
        Exp{std::unexpect, Err{std::move(pred).error()}},
    };
  }
};

} // namespace impl::any_impl

/**
 * @brief Build a parser from a function that matches a single token.
 */
inline constexpr auto satisfy = []<typename F>(F &&fn) static
  requires storable<F> && std::copy_constructible<strip<F>>
{
  return combinate(lift(impl::any_impl::satisfy<strip<F>>{YETI_FWD(fn)}));
};

namespace impl::lit_impl {

template <typename T>
struct err {
  [[nodiscard]] static constexpr auto what() -> std::string_view {
    return "Expected literal={unformattable}";
  }
};

template <std::formattable<char> T>
struct err<T> {
  [[no_unique_address]] T tok;

  [[nodiscard]] constexpr auto what() -> std::string {
    return std::format("Expected literal={}", tok);
  }
};

template <std::copy_constructible T>
struct lit {

  static_assert(std::same_as<T, strip<T>>);

  [[no_unique_address]] T tok;

  template <std::equality_comparable_with<T> U>
  [[nodiscard]] constexpr auto
  operator()(this auto &&self, U &&val) -> std::expected<unit, err<T>> {
    if (val == self.tok) {
      return {};
    }
    return std::expected<unit, err<T>>{std::unexpect, YETI_FWD(self).tok};
  }
};

} // namespace impl::lit_impl

inline constexpr auto lit = []<typename T>(T &&tok) static
  requires storable<T> && std::copy_constructible<strip<T>>
{
  return satisfy(impl::lit_impl::lit<strip<T>>{YETI_FWD(tok)});
};

} // namespace yeti

#endif /* BCC882F7_ED8D_4A23_B468_977755EA6D56 */
