#ifndef CD7DEDCC_2977_4838_B965_15D28F62AE09
#define CD7DEDCC_2977_4838_B965_15D28F62AE09

#include <functional>
#include <variant>

#include "yeti/core/generics.hpp"

namespace yeti {

/**
 * @brief A variant that collapses to `T` if `U == T`.
 *
 * This means you looses the ability to track which is the
 * active type when `U == T`.
 *
 * This provides the only method a variant needs (which `std::`
 * doesent have until C++26 :-O ).
 */
template <typename T, typename U = T>
struct flat_variant final {

  [[no_unique_address]] std::variant<T, U> value;

  constexpr auto visit(this auto &&self, auto &&visitor)
      YETI_HOF(std::visit(YETI_FWD(visitor), YETI_FWD(self).value))
};

template <typename T>
struct flat_variant<T, T> final {

  [[no_unique_address]] T value;

  constexpr auto visit(this auto &&self, auto &&visitor)
      YETI_HOF(std::invoke(YETI_FWD(visitor), YETI_FWD(self).value))
};

} // namespace yeti

#endif /* CD7DEDCC_2977_4838_B965_15D28F62AE09 */
