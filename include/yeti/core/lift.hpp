#ifndef F78764AA_8362_4140_B3F8_666E18A5110D
#define F78764AA_8362_4140_B3F8_666E18A5110D

#include <expected>
#include <functional>
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

template<bool Rep, typename T>
using replace_if  = std::conditional_t<Rep, std::conditional_t<std::same_as<T, never>, never, T>, void> 

// use explicit specializations of this and a common base class that has mute/skip

template<typename P, bool Skip, bool Mute>
struct lifted {


  static_assert(std::same_as<F, strip<F>>);

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  template<typename S>
  using val_t = replace_if<Skip, parse_value_t<P, S>>

  template<typename S>
  using err_t = replace_if<Mute, parse_error_t<P, S>>


  template <typename Self, typename S = type_of<P>>
    requires parser_fn<P, S> && std::same_as<val_t<S>, unit> 
  [[nodiscard]] constexpr auto
  operator()(this Self &&self, S &&stream) -> rebind<P, S, unit, err_t<S>> {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    if (result) {
      return {std::move(rest), {}};
    }

    using Exp = rebind<P, S, unit, void>::expected_type;

    return {std::move(rest), Exp{std::unexpect, std::move(result).error()}};
  }


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
[[nodiscard]] constexpr auto
lift(P &&parser) noexcept(nothrow_storable<P>) -> parser_like<P> auto {
  return impl::parser_lift::lifted<strip<P>>{YETI_FWD(parser)};
}

// ===  === //
// ===  === //
// ===  === //

namespace impl::parser_lift {

template <typename>
struct mute_parser;

template <typename>
struct skip_parser;

template <typename P>
concept missing_skip = !parser_obj_concept::parser_obj_skippable<P>;

template <typename P>
concept missing_mute = !parser_obj_concept::parser_obj_muteable<P>;

template <parser_fn F>
struct lifted final {

  static_assert(std::same_as<F, strip<F>>);

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(YETI_FWD(self).fn.skip())

  // Subsume above.
  template <typename Self>
    requires missing_skip<F>
  [[nodiscard]] constexpr auto skip(this Self &&self)
      YETI_HOF(lifted<skip_parser<F>>{YETI_FWD(self).fn})

  // ===

  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(YETI_FWD(self).fn.mute())

  // Subsume above.
  template <typename Self>
    requires missing_mute<F>
  [[nodiscard]] constexpr auto mute(this Self &&self)
      YETI_HOF(lifted<mute_parser<F>>{YETI_FWD(self).fn})

  // Behave like the `parse_fn` itself.
  template <typename S = type>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))
};

// ===  === //
// ===  === //
// ===  === //

/**
 * @brief This must be publicly inherited and the parent must not shadow `fn`.
 */
template <typename F>
struct skip_mute_base {

  static_assert(std::same_as<F, strip<F>>);

  using type = type_of<F>;

  [[no_unique_address]] F fn;

  // [[nodiscard]] constexpr auto skip(this auto &&self)
  //     YETI_HOF(YETI_FWD(self).fn.skip())

  // [[nodiscard]] constexpr auto mute(this auto &&self)
  //     YETI_HOF(YETI_FWD(self).fn.mute())
};

// ===  === //
// ===  === //
// ===  === //

// TODO: Add_mute and add_skip as separate independant wrapper functions.
// then call lift = add_mute(add_skip(parser))

template <typename P>
struct skip_parser final : skip_mute_base<P> {

  // template <typename Self, typename S = type_of<P>>
  //   requires parser_fn<P, S> && parser_fn<P, S, never, void>
  // [[nodiscard]] constexpr auto operator()(this Self &&self, S &&stream)
  //     YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

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

  // template <typename Self>
  // [[nodiscard]] constexpr auto mute(this Self &&self)
  //     YETI_HOF(lift(YETI_FWD(self)).mute())
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
      return {std::move(rest), {std::move(result).value()}};
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
