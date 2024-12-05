#ifndef F78764AA_8362_4140_B3F8_666E18A5110D
#define F78764AA_8362_4140_B3F8_666E18A5110D

#include <functional>

#include "yeti/core/parser_fn.hpp"
#include <yeti/core/parser.hpp>

/**
 * @brief Tools to lift a `parse_fn` into a `parser`.
 */

namespace yeti {

/**
 * @brief Use like `BOOST_HOF_RETURNS` to define a function/lambda with all the
 * noexcept/requires/decltype specifiers.
 *
 * This macro is not semantically variadic but the ``...`` allows commas in the
 * macro argument.
 */
#define YETI_HOF_RETURNS(...)                                                  \
  noexcept(noexcept(__VA_ARGS__))                                              \
      ->decltype(__VA_ARGS__)                                                  \
    requires requires { __VA_ARGS__; }                                         \
  {                                                                            \
    return __VA_ARGS__;                                                        \
  }

namespace impl::lift {

template <parser_fn F>
struct lift {

  using type = static_type_of<F>;

  F fn;

  template <typename Self>
  auto operator()(Self &&self, std::string_view sv)
      YETI_HOF_RETURNS(std::invoke(std::forward<Self>(self).fn, sv))
};

} // namespace impl::lift

} // namespace yeti

#endif /* F78764AA_8362_4140_B3F8_666E18A5110D */
