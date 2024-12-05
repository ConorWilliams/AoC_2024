#ifndef B17D04A5_6459_4D9B_8B72_E637613D1DCD
#define B17D04A5_6459_4D9B_8B72_E637613D1DCD

#include <string_view>
#include <utility>

/**
 * @brief Some of the most fundamental parsers.
 */

namespace yeti {

/**
 * @brief A unit type that signals a value/error that cannot be produced.
 */
struct never {

  never() = delete;

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
    std::unreachable();
  }
};

} // namespace yeti

#endif /* B17D04A5_6459_4D9B_8B72_E637613D1DCD */
