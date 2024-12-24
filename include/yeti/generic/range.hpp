#ifndef BCC882F7_ED8D_4A23_B468_977755EA6D56
#define BCC882F7_ED8D_4A23_B468_977755EA6D56

#include <expected>
#include <format>
#include <iterator>
#include <ranges>
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
concept recombinant_input_range =
    std::ranges::input_range<T> && recombinant_range<T>;

template <typename T>
concept recombinant_forward_range =
    std::ranges::forward_range<T> && recombinant_range<T>;

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

template <typename F, typename I>
concept expected_invocable_help =
    std::indirectly_unary_invocable<F, I> &&
    specialization_of<std::indirect_result_t<F, I>, std::expected> &&
    error<typename std::indirect_result_t<F, I>::error_type>;

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
  constexpr auto
  operator()(this Self &&self, S &&stream) -> specialization_of<result> auto {

    using I = std::ranges::iterator_t<S>;
    using R = std::indirect_result_t<forward_fn_t<Self>, I>;

    static_assert(specialization_of<R, std::expected>);

    using V = R::value_type;
    using E = R::error_type;

    using Err = flat_variant<eos, E>;
    using Res = resulting_t<S, V, Err>;
    using Exp = Res::expected_type;

    auto beg = std::ranges::begin(stream);
    auto end = std::ranges::end(stream);

    if (beg == end) {
      return Res{YETI_FWD(stream), Exp{std::unexpect, eos{}}};
    }

    auto pred = YETI_FWD(self).fn(beg);

    beg = std::next(std::move(beg));

    if (pred) {
      return Res{{std::move(beg), std::move(end)}, {std::move(pred).value()}};
    }

    return Res{
        {std::move(beg), std::move(end)},
        Exp{std::unexpect, Err{std::move(pred).error()}},
    };
  }
};

} // namespace impl::any_impl

/**
 * @brief Build a parser from a function that matches a single token.
 */
inline constexpr auto satisfy = []<typename F>(F &&fn) static {
  return combinate(lift(impl::any_impl::satisfy<strip<F>>{YETI_FWD(fn)}));
};

} // namespace yeti

#endif /* BCC882F7_ED8D_4A23_B468_977755EA6D56 */
