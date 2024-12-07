#ifndef E0903E10_DE5D_4348_8AFF_659AF849A3A2
#define E0903E10_DE5D_4348_8AFF_659AF849A3A2

#include <concepts>
#include <type_traits>

#include "yeti/core/generics.hpp"

namespace yeti {

namespace impl {

template <typename>
struct type_impl : std::type_identity<void> {};

template <typename T>
  requires requires { typename T::type; }
struct type_impl<T> : std::type_identity<typename T::type> {};

} // namespace impl

/**
 * @brief Fetch the `::type` member, or `void` if not present.
 */
template <typename T>
using type_of = impl::type_impl<strip<T>>::type;

/**
 * @brief Test if a typed defines a non-void `::type` member.
 */
template <typename T>
concept typed = different_from<type_of<T>, void>;

} // namespace yeti

#endif /* E0903E10_DE5D_4348_8AFF_659AF849A3A2 */
