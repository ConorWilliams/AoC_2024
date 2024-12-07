#ifndef B17D04A5_6459_4D9B_8B72_E637613D1DCD
#define B17D04A5_6459_4D9B_8B72_E637613D1DCD

#include <expected>
#include <ranges>
#include <string_view>
#include <utility>

#include "yeti/core.hpp"
#include "yeti/core/generics.hpp"

/**
 * @brief Some of the most fundamental parsers.
 */

namespace yeti {

/**
 * @brief Use this as placeholder for the return of `skip`/`mute` methods.
 *
 * The methods of this placeholder are not defined, this is for
 * satisfying the type system not for implementing the methods.
 */
struct todo {
  template <typename T>
  static constexpr auto operator()(T &&) -> resulting<T, unit, unit>;
  static constexpr auto skip() -> todo;
  static constexpr auto mute() -> todo;
};

// ===  === //
// ===  === //
// ===  === /

/**
 * @brief A unit type that signals a value/error that cannot be produced.
 */
struct never : impl::mixin_equal {

  never() = delete ("This type should never be constructed.");

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
    std::unreachable();
  }
};

// ===  === //
// ===  === //
// ===  === //

namespace impl::fail_impl {

struct fail {
  template <storable S>
  static constexpr auto operator()(S &&stream) noexcept(nothrow_storable<S>)
      -> resulting<S, never, unit> {
    return {YETI_FWD(stream), {std::unexpect, unit{}}};
  }
};

} // namespace impl::fail_impl

/**
 * @brief This parser fails without consuming any input.
 */
inline constexpr auto fail = combinate(lift(impl::fail_impl::fail{}));

// ===  === //
// ===  === //
// ===  === //

namespace impl::pure_impl {

struct pure {
  template <storable S>
  static constexpr auto operator()(S &&stream) noexcept(nothrow_storable<S>)
      -> resulting<S, unit, never> {
    return {YETI_FWD(stream), {}};
  }
};

} // namespace impl::pure_impl

/**
 * @brief The pure parser always succeeds without consuming any input.
 */
inline constexpr auto pure = combinate(lift(impl::pure_impl::pure{}));

// ===  === //
// ===  === //
// ===  === //

namespace impl::eos_impl {

struct eos {

  static constexpr auto skip() noexcept -> eos { return {}; }

  static constexpr auto mute() noexcept -> eos { return {}; }

  template <storable S>
  static constexpr auto operator()(S &&stream) -> resulting<S, unit, unit>
    requires requires { std::ranges::empty(YETI_FWD(stream)); }
  {
    if (std::ranges::empty(stream)) {
      return {YETI_FWD(stream), {}};
    }
    return {YETI_FWD(stream), {std::unexpect, unit{}}};
  }
};

} // namespace impl::eos_impl

/**
 * @brief The EOS parser succeeds if the input stream is empty.
 *
 * This uses the `std::ranges::empty` function to check if the stream is empty.
 *
 * This parser does not consume any input.
 */
inline constexpr auto eos = combinate(impl::eos_impl::eos{});

} // namespace yeti

#endif /* B17D04A5_6459_4D9B_8B72_E637613D1DCD */
