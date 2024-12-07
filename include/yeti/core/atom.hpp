#ifndef B17D04A5_6459_4D9B_8B72_E637613D1DCD
#define B17D04A5_6459_4D9B_8B72_E637613D1DCD

#include <string_view>
#include <utility>

#include "yeti/core/combinator.hpp"
#include "yeti/core/parser.hpp"

/**
 * @brief Some of the most fundamental parsers.
 */

namespace yeti {

/**
 * @brief A unit type that signals a value/error that cannot be produced.
 */
struct never : impl::mixin_equal {

  never() = delete ("This type should never be constructed.");

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
    std::unreachable();
  }
};

/**
 * @brief Use this as placeholder for the return of `skip`/`mute` methods.
 *
 * The methods of this placeholder are not defined, this is for
 * satisfying the type system not for implementing the methods.
 */
struct todo {
  template <typename T>
  static constexpr auto operator()(T) -> result<T, unit, unit>;
  static constexpr auto skip() -> todo;
  static constexpr auto mute() -> todo;
};

} // namespace yeti

#endif /* B17D04A5_6459_4D9B_8B72_E637613D1DCD */
