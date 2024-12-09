#ifndef C769280B_5362_4F45_9813_D03B9AAAD3C7
#define C769280B_5362_4F45_9813_D03B9AAAD3C7

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
  [[nodiscard]] static constexpr auto what() noexcept -> std::string_view {
    return {};
  }
};

// ===  === //
// ===  === //
// ===  === /

/**
 * @brief The bottom type in yeti.
 */
struct never : impl::mixin_equal {

  never() = delete ("This type should never be constructed.");

  [[noreturn]] static constexpr auto what() noexcept -> std::string_view {
    std::unreachable();
  }
};

// TODO: Add never handling as s special case to parser definition, i.e. alow
// skip to be a parser that returns ever
// TODO: Remove templating from the noop and fail once above is done

} // namespace yeti

#endif /* C769280B_5362_4F45_9813_D03B9AAAD3C7 */