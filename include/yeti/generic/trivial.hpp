#ifndef B17D04A5_6459_4D9B_8B72_E637613D1DCD
#define B17D04A5_6459_4D9B_8B72_E637613D1DCD

#include <expected>
#include <ranges>
#include <string_view>
#include <utility>

#include "yeti/core.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser_fn.hpp"

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
struct [[deprecated]] todo {

  static constexpr auto skip() -> todo;
  static constexpr auto mute() -> todo;

  template <typename T>
  static constexpr auto operator()(T) -> result<T, never, never>;
};

// ===  === //
// ===  === //
// ===  === //

namespace impl::fail_impl {

struct err {
  static constexpr auto what() noexcept -> std::string_view {
    return "The fail parser was invoked.";
  }
};

template <either<err, unit> E = err>
struct fail {

  static constexpr auto skip() noexcept -> fail<E> { return {}; }
  static constexpr auto mute() noexcept -> fail<unit> { return {}; }

  // clang-format off

  template <storable S>
  static constexpr auto operator()(S &&stream) 
  noexcept(nothrow_storable<S>) -> resulting_t<S, never, E> {
    return {YETI_FWD(stream), std::expected<never, E>{std::unexpect, E{}}};
  }

  // clang-format on
};

} // namespace impl::fail_impl

/**
 * @brief This parser fails without consuming any input.
 */
inline constexpr auto fail = combinate(impl::fail_impl::fail<>{});

// ===  === //
// ===  === //
// ===  === //

namespace impl::pure_impl {

struct pure {

  static constexpr auto skip() noexcept -> pure { return {}; }
  static constexpr auto mute() noexcept -> pure { return {}; }

  // clang-format off

  template <storable S>
  static constexpr auto operator()(S &&stream)
  noexcept(nothrow_storable<S>) -> resulting_t<S, unit, never> {
    return {YETI_FWD(stream), {}};
  }

  // clang-format on
};

} // namespace impl::pure_impl

/**
 * @brief The pure parser always succeeds without consuming any input.
 */
inline constexpr auto pure = combinate(impl::pure_impl::pure{});

// ===  === //
// ===  === //
// ===  === //

namespace impl::eos_impl {

struct err {
  static constexpr auto what() noexcept -> std::string_view {
    return "Expecting end of stream.";
  }
};

template <typename E = err>
struct eos {

  static constexpr auto skip() noexcept -> eos { return {}; }
  static constexpr auto mute() noexcept -> eos<unit> { return {}; }

  template <storable S>
  static constexpr auto operator()(S &&stream) -> resulting_t<S, unit, E>
    requires requires { std::ranges::empty(YETI_FWD(stream)); }
  {
    if (std::ranges::empty(stream)) {
      return {YETI_FWD(stream), {}};
    }

    return {
        YETI_FWD(stream),
        std::expected<unit, E>{std::unexpect, E{}},
    };
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
inline constexpr auto eos = combinate(impl::eos_impl::eos<>{});

} // namespace yeti

#endif /* B17D04A5_6459_4D9B_8B72_E637613D1DCD */
