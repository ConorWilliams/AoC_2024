#ifndef F78764AA_8362_4140_B3F8_666E18A5110D
#define F78764AA_8362_4140_B3F8_666E18A5110D

#include <expected>
#include <functional>
#include <type_traits>
#include <utility>

#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"
#include "yeti/core/parser_obj.hpp"
#include "yeti/core/rebind.hpp"
#include <yeti/core/parser.hpp>

/**
 * @brief Tools to lift a `parse_fn` into a `parser`.
 */

namespace yeti {

namespace impl::parser_lift {
template <typename F, bool Skip = false, bool Mute = false>
struct lifted;
}

/**
 * @brief Short-circuit for parsers.
 */
template <typename P>
  requires parser<P>
[[nodiscard]] constexpr auto lift(P &&parser) noexcept -> P && {
  return YETI_FWD(parser);
}

/**
 * @brief Transform a `parse_fn` into a `parser`.
 */
template <typename P>
  requires parser_fn<P>
[[nodiscard]] constexpr auto
lift(P &&parser) noexcept(nothrow_storable<P>) -> parser_like<P> auto {
  return impl::parser_lift::lifted<strip<P>>{YETI_FWD(parser)};
}

// ===  === //
// ===  === //
// ===  === //

namespace impl::parser_lift {

template <typename F, bool Skip, bool Mute>
struct lifted_base {

  static_assert(std::same_as<F, strip<F>>);

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(lifted<F, true, Mute>{YETI_FWD(self).fn})

  template <typename Self>
    requires Skip
  [[nodiscard]] constexpr auto skip(this Self &&self) -> Self && {
    return YETI_FWD(self);
  }

  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(lifted<F, Skip, true>{YETI_FWD(self).fn})

  template <typename Self>
    requires Mute
  [[nodiscard]] constexpr auto mute(this Self &&self) -> Self && {
    return YETI_FWD(self);
  }
};

/**
 * @brief Act exactly like the input `parse_fn`.
 */
template <typename F>
struct lifted<F, false, false> : lifted_base<F, false, false> {
  template <typename S = type_of<F>>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))
};

template <typename F>
struct lifted<F, true, false> : lifted_base<F, true, false> {

  template <typename Self, typename S = type_of<F>>
    requires parser_fn<F, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<F, S, unit, void> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {}};
    }

    using Exp = rebind<F, S, unit, void>::expected_type;

    return {std::move(rest), Exp{std::unexpect, std::move(result).error()}};
  }

  template <typename Self, typename S = type_of<F>>
    requires parser_fn<F, S> && std::same_as<parse_value_t<F, S>, never>
  [[nodiscard]] constexpr auto operator()(this Self &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))
};

template <typename F>
struct lifted<F, false, true> : lifted_base<F, false, true> {

  template <typename Self, typename S = type_of<F>>
    requires parser_fn<F, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<F, S, void, unit> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {std::move(result).value()}};
    }

    using Exp = rebind<F, S, void, unit>::expected_type;

    return {std::move(rest), Exp{std::unexpect, unit{}}};
  }

  template <typename Self, typename S = type_of<F>>
    requires parser_fn<F, S> && std::same_as<parse_error_t<F, S>, never>
  [[nodiscard]] constexpr auto operator()(this Self &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))
};

template <typename T>
using never_or_unit = std::conditional_t<std::same_as<T, never>, never, unit>;

template <typename F>
struct lifted<F, true, true> : lifted_base<F, true, true> {

  template <typename S>
  using val_t = never_or_unit<parse_value_t<F, S>>;

  template <typename S>
  using err_t = never_or_unit<parse_error_t<F, S>>;

  template <typename Self, typename S = type_of<F>>
    requires parser_fn<F, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<F, S, val_t<S>, err_t<S>> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      if constexpr (std::same_as<val_t<S>, never>) {
        return {std::move(rest), std::move(result).value()};
      } else {
        return {std::move(rest), {}};
      }
    }

    using Exp = std::expected<val_t<S>, err_t<S>>;

    if constexpr (std::same_as<err_t<S>, never>) {
      return {std::move(rest), Exp{std::unexpect, std::move(result).error()}};
    } else {
      return {std::move(rest), Exp{std::unexpect, unit{}}};
    }
  }
};

} // namespace impl::parser_lift

} // namespace yeti

#endif /* F78764AA_8362_4140_B3F8_666E18A5110D */
