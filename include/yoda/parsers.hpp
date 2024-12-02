#ifndef A75B5447_AA14_4DF3_8767_82A33677EC06
#define A75B5447_AA14_4DF3_8767_82A33677EC06

#include <concepts>
#include <expected>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <version>

#include "yoda/combinators.hpp"
#include "yoda/core.hpp"

namespace yoda {

/**
 * @brief The noop parser, consumes nothing.
 */
constexpr auto noop = [](std::string_view sv) -> result<std::monostate> {
  return {{}, sv};
};

/**
 * @brief The any parser, consumes any character.
 */
constexpr auto any = [](std::string_view sv) -> result<char> {
  if (sv.empty()) {
    return {detail::err("Expected any character but got EoF"), {}};
  }
  return {sv[0], sv.substr(1)};
};

/**
 * @brief The literal parser, consumes a specific character.
 */
constexpr auto lit = [](char c) {
  return [c](std::string_view sv) -> result<char> {
    if (sv.empty()) {
      return {detail::err("Expected '{}' but got EoF", c), {}};
    } else if (sv[0] == c) {
      return {c, sv.substr(1)};
    }
    return {detail::err("Expected '{}' but got '{}'", c, sv[0]), sv};
  };
};

/**
 * @brief The end-of-file parser, consumes only the end of the input.
 */
constexpr auto eof = [](std::string_view sv) -> result<std::monostate> {
  if (sv.empty()) {
    return {{}, {}};
  }
  return {detail::err("Expected EoF but got '{}'", sv), sv};
};

} // namespace yoda

#endif /* A75B5447_AA14_4DF3_8767_82A33677EC06 */
