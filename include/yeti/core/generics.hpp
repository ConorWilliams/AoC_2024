#ifndef DC2FEF19_9F77_4C43_B38F_7FF83C9748EA
#define DC2FEF19_9F77_4C43_B38F_7FF83C9748EA

#include <concepts>

/**
 * @brief Self contained generic utilities not tied to yeti.
 */

namespace yeti {

// clang-format off

/**
 * @brief Use like `BOOST_HOF_RETURNS` to define a function/lambda with all
 * the noexcept/requires/decltype specifiers.
 *
 * This macro is not semantically variadic but the ``...`` allows commas in
 * the macro argument.
 */
#define YETI_HOF(...)                                                          \
  noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__)                     \
    requires requires { __VA_ARGS__; }                                         \
  {                                                                            \
    return __VA_ARGS__;                                                        \
  }

// clang-format on

/**
 * @brief Test if a type is any of the given types.
 */
template <typename T, typename... Args>
concept either = (std::same_as<T, Args> || ...);

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