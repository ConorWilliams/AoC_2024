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

struct dummy {
  template <typename T>
  static constexpr auto operator()(T) -> result<T, unit, unit>;
  static constexpr auto skip() -> dummy;
  static constexpr auto mute() -> dummy;
};

namespace impl::parser_lift {

template <parser_fn F>
struct lifted;

} // namespace impl::parser_lift

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
[[nodiscard]] constexpr auto lift(P parser) noexcept -> parser_like<P> auto {
  return impl::parser_lift::lifted<P>{std::move(parser)};
}

// ===  === //
// ===  === //
// ===  === //

namespace impl::parser_lift {

template <typename F>
// requires parser_fn<F>
struct mute_parser;

template <typename F>
struct skip_parser;

template <typename P>
concept missing_skip = !parser_obj_concept::parser_obj_skippable<P>;

template <typename P>
concept missing_mute = !parser_obj_concept::parser_obj_muteable<P>;

template <parser_fn F>
struct lifted final {

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  // clang-format off

  template <typename Self>
  [[nodiscard]] constexpr auto skip(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.skip())

  // Subsume above.
  template <typename Self>
    requires missing_skip<F>
  [[nodiscard]] constexpr auto skip(this Self &&self) 
      YETI_HOF(skip_parser<lifted>{YETI_FWD(self)})

  // ===

  template <typename Self>
  [[nodiscard]] constexpr auto mute(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.mute())


  // Subsume above.
  template <typename Self>
    requires missing_mute<F>
  [[nodiscard]] constexpr auto mute(this Self &&self) 
      YETI_HOF(dummy{})

  // Behave like the `parse_fn` itself.

  template <typename Self, typename S = type>
  [[nodiscard]] constexpr auto operator()(this Self &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  // clang-format on
};

// ===  === //
// ===  === //
// ===  === //

/**
 * @brief This must be publicly inherited and the parent must not shadow `fn`.
 */
template <typename F>
struct skip_mute_base {

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  // clang-format off

  template <typename Self>
    // requires parser_obj_skippable<F>
  [[nodiscard]] constexpr auto skip(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.skip())

  template <typename Self>
    // requires parser_obj_muteable<F>
  [[nodiscard]] constexpr auto mute(this Self &&self) 
      YETI_HOF(std::forward<Self>(self).fn.skip())

  // clang-format on
};

// ===  === //
// ===  === //
// ===  === //

template <typename P>
struct skip_parser final : skip_mute_base<P> {

  template <typename Self, typename S = type_of<P>>
    requires parser_fn<P, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<P, S, unit, void> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {}};
    }

    using Exp = rebind<P, S, unit, void>::expected_type;

    return {std::move(rest), Exp{std::unexpect, std::move(result).error()}};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto skip(this Self &&self) -> Self && {
    return YETI_FWD(self);
  }
};

// ===  === //
// ===  === //
// ===  === //

template <typename P>
struct mute_parser final : skip_mute_base<P> {

  template <typename Self, typename S = type_of<P>>
    requires parser_fn<P, S>
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<P, S, void, unit> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {}};
    }

    using Exp = rebind<P, S, void, unit>::expected_type;

    return {std::move(rest), Exp{std::unexpect, unit{}}};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto mute(this Self &&self) -> Self && {
    return YETI_FWD(self);
  }
};

} // namespace impl::parser_lift

} // namespace yeti

#endif /* F78764AA_8362_4140_B3F8_666E18A5110D */
