#ifndef CD7DEDCC_2977_4838_B965_15D28F62AE09
#define CD7DEDCC_2977_4838_B965_15D28F62AE09

#include <functional>
#include <type_traits>
#include <variant>

#include "yeti/core/blessed.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/result.hpp"

namespace yeti {

namespace impl::variant {

template <typename... Ts>
struct flat_variant;

template <typename T>
struct flat_variant<T> final {

  [[no_unique_address]] T value;

  template <typename Self>
    requires error<T>
  [[nodiscard]] constexpr auto what(this Self &&self)
      YETI_HOF(YETI_FWD(self).value.what())

  constexpr auto visit(this auto &&self, auto &&visitor)
      YETI_HOF(std::invoke(YETI_FWD(visitor), YETI_FWD(self).value))
};

template <typename... T>
struct flat_variant final {

  [[no_unique_address]] std::variant<T...> value;

  template <typename Self>
    requires (error<T> && ...)
  [[nodiscard]] constexpr auto what(this Self &&self)
      YETI_HOF(YETI_FWD(self).visit([](auto &&val) static -> std::string {
        return YETI_FWD(val).what();
      }))

  template <typename Self>
    requires (error_view<T> && ...)
  [[nodiscard]] constexpr auto what(this Self &&self)
      YETI_HOF(YETI_FWD(self).visit([](auto &&val) static -> std::string_view {
        return YETI_FWD(val).what();
      }))

  constexpr auto visit(this auto &&self, auto &&visitor)
      YETI_HOF(std::visit(YETI_FWD(visitor), YETI_FWD(self).value))
};

template <typename...>
struct list;

template <typename, typename...>
struct merge_flat;

// Terminals

template <>
struct merge_flat<list<>> : std::type_identity<never> {};

template <typename... U>
struct merge_flat<list<U...>> : std::type_identity<flat_variant<U...>> {};

// Recursive

// never -> flat_variant<>
template <typename... U, typename... Ts>
struct merge_flat<list<U...>, never, Ts...> : merge_flat<list<U...>, Ts...> {};

// prune repeats
template <typename... U, typename T, typename... Ts>
  requires (std::same_as<T, U> || ...)
struct merge_flat<list<U...>, T, Ts...> : merge_flat<list<U...>, Ts...> {};

// Expand variant
template <typename... U, typename... Us, typename... Ts>
struct merge_flat<list<U...>, flat_variant<Us...>, Ts...>
    : merge_flat<list<U...>, Us..., Ts...> {};

// Unique == OK
template <typename... U, typename T, typename... Ts>
struct merge_flat<list<U...>, T, Ts...> : merge_flat<list<U..., T>, Ts...> {};

// Specializations for concat to speed up matching

template <typename... Ls, typename... Ts>
struct merge_flat<list<>, flat_variant<Ls...>, Ts...>
    : merge_flat<list<Ls...>, Ts...> {};

template <typename L, typename... Rs, typename... Ts>
struct merge_flat<list<>, L, flat_variant<Rs...>, Ts...>
    : merge_flat<list<Rs...>, L, Ts...> {};

template <typename... Ls, typename... Rs, typename... Ts>
struct merge_flat<list<>, flat_variant<Ls...>, flat_variant<Rs...>, Ts...>
    : merge_flat<list<Ls...>, Rs..., Ts...> {};

// Helper

template <typename... Ts>
using merge_flat_t = merge_flat<list<>, Ts...>::type;

} // namespace impl::variant

/**
 * @brief A variant that removes identical types.
 *
 * This means you looses the ability to track which is the
 * active degenerate members.
 *
 * This provides the only method a variant needs (which `std::`
 * doesent have until C++26 :-O ).
 *
 * In addition if all the Ts are `error` then the flat variant
 * satisfies the `error` concept.
 */
template <typename... T>
using flat_variant = impl::variant::merge_flat_t<T...>;

// static_assert(std::same_as<T, impl::variant::flat_variant<int, float>>);

} // namespace yeti

#endif /* CD7DEDCC_2977_4838_B965_15D28F62AE09 */
