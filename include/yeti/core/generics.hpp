#ifndef DC2FEF19_9F77_4C43_B38F_7FF83C9748EA
#define DC2FEF19_9F77_4C43_B38F_7FF83C9748EA

#include <concepts>

/**
 * @brief Self contained generic utilities not tied to yeti.
 */

namespace yeti {

/**
 * @brief Use like `BOOST_HOF_RETURNS` to define a function/lambda with all
 * the noexcept/decltype specifiers.
 *
 * This implementation deliberately omits the `requires` clause to allow
 * for subsumption by more-constrained implementations via a concept. This
 * does not make them any less SFINAE friendly due to the return specification.
 *
 * This macro is not semantically variadic but the ``...`` allows commas in
 * the macro argument.
 */
#define YETI_HOF(...)                                                          \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { return __VA_ARGS__; }

/**
 * @brief Love to hate it, hate to love it, love to have it.
 */
#define YETI_FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

/**
 * @brief Test if a type is any of the given types.
 */
template <typename T, typename... Args>
concept either = (std::same_as<T, Args> || ...);

/**
 * @brief Test if two types are different.
 */
template <typename T, typename U>
concept different_from = !std::same_as<T, U>;

template <typename T>
concept pure_void = std::same_as<void, T>;

template <typename T>
concept not_void = different_from<void, T>;

template <typename T, typename U>
concept same_as_stripped =
    std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

namespace impl {

template <typename, template <typename...> typename>
struct specialization_of_concept : std::false_type {};

template <typename... Ts, template <typename...> typename U>
struct specialization_of_concept<U<Ts...>, U> : std::true_type {};

} // namespace impl

/**
 * @brief Test if a type is a (type) specialization of a template.
 */
template <typename T, template <typename...> typename U>
concept specialization_of = impl::specialization_of_concept<T, U>::value;

} // namespace yeti

#endif /* DC2FEF19_9F77_4C43_B38F_7FF83C9748EA */
