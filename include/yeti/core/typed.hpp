#ifndef E0903E10_DE5D_4348_8AFF_659AF849A3A2
#define E0903E10_DE5D_4348_8AFF_659AF849A3A2

#include <concepts>
#include <type_traits>

#include "yeti/core/generics.hpp"

namespace yeti {

namespace impl {

template <typename>
struct static_type_impl : std::type_identity<void> {};

template <typename T>
  requires requires { typename T::type; }
struct static_type_impl<T> : std::type_identity<typename T::type> {};

} // namespace impl

/**
 * @brief Fetch the `::type` member, or `void` if not present.
 */
template <typename T>
using static_type_of = impl::static_type_impl<std::remove_cvref_t<T>>::type;

/**
 * @brief Test if a typed defines a non-void `::type` member.
 */
template <typename T>
concept typed = different_from<static_type_of<T>, void>;

namespace impl {

/**
 * @brief If `S` is void then use the static type of `P`.
 */
template <typename S, typename P>
using else_static = std::conditional_t<std::is_void_v<S>, static_type_of<P>, S>;

} // namespace impl

} // namespace yeti

#endif /* E0903E10_DE5D_4348_8AFF_659AF849A3A2 */