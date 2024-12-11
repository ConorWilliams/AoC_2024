#ifndef A8E7277C_74F1_48C2_82B8_BB67ED9A0FFF
#define A8E7277C_74F1_48C2_82B8_BB67ED9A0FFF

#include <concepts>
#include <expected>
#include <string>
#include <string_view>

#include "yeti/core/generics.hpp"

namespace yeti {

/**
 * @brief A yeti error is produced on-demand via the `what` method.
 */
template <typename E>
concept error = std::movable<E> && requires (E e) {
  { e.what() } -> either<std::string, std::string_view>;
};

template <typename E>
concept error_view = error<E> && requires (E e) {
  { e.what() } -> std::same_as<std::string_view>;
};

/**
 * @brief The result of invoking a yeti parser.
 *
 * A parser can succeed or fail, regardless it returns the unconsumed input.
 *
 * TIP: Try destructuring the result type to get the value and the rest.
 *
 * @tparam T The type of the result of the parse.
 * @tparam E The type of the error of the parse.
 * @tparam S The type of stream that the parser consumes.
 */
template <std::movable S, std::movable T, error E>
struct result {

  using value_type = T;
  using error_type = E;

  using unparsed_type = S;
  using expected_type = std::expected<T, E>;

  [[no_unique_address]] unparsed_type unparsed; ///< Unconsumed input.
  [[no_unique_address]] expected_type expected; ///< The result of the parse.

  /**
   * @brief Compare two results for equality.
   */
  friend constexpr auto
  operator==(result const &, result const &) -> bool = default;

  /**
   * @brief Test if the parse was successful.
   */
  constexpr explicit operator bool() const noexcept {
    return expected.has_value();
  }
};

/**
 * @brief An alias for `result` that strips `S`.
 */
template <typename S, std::movable T, std::movable E>
  requires std::movable<strip<S>>
using resulting_t = result<strip<S>, T, E>;

} // namespace yeti

#endif /* A8E7277C_74F1_48C2_82B8_BB67ED9A0FFF */
