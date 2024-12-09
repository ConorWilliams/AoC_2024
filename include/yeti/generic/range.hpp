#ifndef BCC882F7_ED8D_4A23_B468_977755EA6D56
#define BCC882F7_ED8D_4A23_B468_977755EA6D56

#include <format>
#include <iterator>
#include <utility>
#include <variant>

#include <concepts>

#include "yeti/core.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"
#include "yeti/generic/trivial.hpp"

namespace yeti {

namespace impl::any_impl {

template <error Err>
struct err final {

  [[no_unique_address]] flat_variant<eos_impl::err, Err> err;

  [[nodiscard]] constexpr auto
  what(this auto &&self) noexcept -> std::string_view {
    return YETI_FWD(self).err.visit([](auto &&e) {
      return std::format("Satisfy, predicate failed with: {}", e.what());
    });
  }
};

/**
 * @brief The result of accessing `t.fn` when `t` is a `T`.
 */
template <typename T>
using fwd_fn_t = decltype(std::declval<T>().fn);

// template <typename T, typename... Args>
// concept inner_indirectly_invocable =

/**
 * @brief The result of invoking `t.fn` with `Args...` when `t` is a `T`.
 */
template <typename T, typename... Args>
using fn_result_t = std::invoke_result_t<fwd_fn_t<T>, Args...>;

template <typename F>
struct satisfy final {

  static_assert(std::same_as<F, strip<F>>);

  [[no_unique_address]] F fn;

  [[nodiscard]] constexpr satisfy() noexcept = default;

  template <typename Self, storable S>
  // requires that fn is invocable with S to an expected<unit, E>
  constexpr auto
  operator()(this Self &&self, S &&stream) -> specialization_of<result> auto {

    using I = std::ranges::iterator_t<S>;
    using V = std::iter_value_t<I>;

    using R = fn_result_t<Self, V &>;

    static_assert(specialization_of<R, std::expected>);

    using Err = typename R::error_type;
    using Result = resulting_t<V, V, err<Err>>;
    using Exp = Result::expected_type;

    auto beg = std::ranges::begin(stream);
    auto end = std::ranges::end(stream);

    if (beg == end) {
      return Result{YETI_FWD(stream), Exp{std::unexpect, Err{}}};
    }

    V val = *beg;
    ++beg;
    auto pred = YETI_FWD(self).fn(val);

    if (!pred) {
      return Result{
          strip<S>{std::move(beg), std::move(end)},
          Exp{std::unexpect, Err{{std::move(pred).error()}}},
      };
    }

    return Result{
        strip<S>{std::move(beg), std::move(end)},
        {std::move(val)},
    };
  }
};

} // namespace impl::any_impl

} // namespace yeti

#endif /* BCC882F7_ED8D_4A23_B468_977755EA6D56 */
