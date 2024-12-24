#ifndef C769280B_5362_4F45_9813_D03B9AAAD3C7
#define C769280B_5362_4F45_9813_D03B9AAAD3C7

#include <format>
#include <string_view>
#include <utility>

namespace yeti {

namespace impl {

struct mixin_equal {
  friend constexpr auto
  operator==(mixin_equal const &, mixin_equal const &) -> bool = default;
};

} // namespace impl

/**
 * @brief In yeti THE unit type is the regular-void for both values and errors.
 */
struct unit : impl::mixin_equal {
  [[nodiscard]] static constexpr auto what() noexcept -> std::string_view { return "()"; }
};

// ===  === //
// ===  === //
// ===  === /

/**
 * @brief The bottom type in yeti.
 */
struct never : impl::mixin_equal {

  never() = delete ("This type should never be constructed.");

  // Defing this with UB allows aggressive optimizations.

  never(never const &) { std::unreachable(); }

  [[noreturn]] auto operator=(never const &) -> never & { std::unreachable(); }

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
    std::unreachable(), "!";
  }

  ~never() { std::unreachable(); }
};

} // namespace yeti

template <>
struct std::formatter<yeti::unit> : std::formatter<std::string_view> {
  auto format(yeti::unit const &, std::format_context &context) const {
    return std::formatter<std::string_view>::format("()", context);
  }
};

template <>
struct std::formatter<yeti::never> : std::formatter<char> {
  auto format(yeti::never const &, std::format_context &context) const {
    return std::formatter<char>::format('!', context);
  }
};

#endif /* C769280B_5362_4F45_9813_D03B9AAAD3C7 */
