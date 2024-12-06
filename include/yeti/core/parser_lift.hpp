#ifndef F78764AA_8362_4140_B3F8_666E18A5110D
#define F78764AA_8362_4140_B3F8_666E18A5110D

#include <expected>
#include <functional>

#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"
#include "yeti/core/parser_obj.hpp"
#include <yeti/core/parser.hpp>

/**
 * @brief Tools to lift a `parse_fn` into a `parser`.
 */

namespace yeti {

namespace impl::parser_lift {
template <parser_fn F>
struct lift;
}

/**
 * @brief Short-circuit for parsers.
 */
template <typename P>
  requires parser<P>
constexpr auto lift(P &&parser) noexcept -> P && {
  return std::forward<P>(parser);
}

/**
 * @brief Transform a `parse_fn` into a `parser`.
 */
template <typename P>
  requires parser_fn<P>
constexpr auto lift(P &&parser) -> parser_like<P> auto {
  return impl::parser_lift::lift<P>{std::forward<P>(parser)};
}

// ===  === //
// ===  === //
// ===  === //

namespace impl::parser_lift {

template <typename P, typename S>
using skip_result = result<std::decay_t<S>, unit, parse_error_t<P, S>>;

template <specialization_of<lift> P>
struct skip_parser : P {

  template <typename Self, typename S = P::type>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> skip_result<P, S> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {}};
    }

    using Exp = std::expected<unit, parse_error_t<P, S>>;

    return {std::move(rest), Exp{std::unexpect, std::move(result).error()}};
  }
};

template <typename P, typename S>
using mute_result = result<std::decay_t<S>, parse_value_t<P, S>, unit>;

template <specialization_of<lift> P>
struct mute_parser : P {
  template <typename Self, typename S = P::type>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> mute_result<P, S> {}
};

struct dummy {
  template <typename T>
  static constexpr auto operator()(T) -> result<T, unit, unit>;
  static constexpr auto skip() -> dummy;
  static constexpr auto mute() -> dummy;
};

// ===  === //
// ===  === //
// ===  === //

// Components of the parser_obj concept.

using parser_obj_concept::parser_obj_muteable;

using parser_obj_concept::parser_obj_skippable;

template <parser_fn F>
struct lift {

  using type = static_type_of<F>;

  F fn;

  // Fallbacks are the naive implementation.

  template <typename Self>
  [[nodiscard]] constexpr auto skip(this Self &&self) -> skip_parser<lift> {
    return {YETI_FWD(self)};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto mute(this Self &&self) -> mute_parser<lift> {
    return {YETI_FWD(self)};
  }

  // clang-format off

  template <typename Self>
    requires parser_obj_skippable<F>
  [[nodiscard]] constexpr auto skip(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.skip())

  template <typename Self>
    requires parser_obj_skippable<F>
  [[nodiscard]] constexpr auto mute(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.skip())

  template <typename Self, typename S = type>
  [[nodiscard]] constexpr auto operator()(this Self &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  // clang-format on
};

#undef FWD

} // namespace impl::parser_lift

} // namespace yeti

#endif /* F78764AA_8362_4140_B3F8_666E18A5110D */
