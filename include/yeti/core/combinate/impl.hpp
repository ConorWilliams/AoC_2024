#ifndef CA19C867_8391_4381_AA78_404CA569C1F7
#define CA19C867_8391_4381_AA78_404CA569C1F7

#include <concepts>
#include <functional>

#include "yeti/core/generics.hpp"
#include "yeti/core/typed.hpp"
#include <yeti/core/parser.hpp>

namespace yeti {

namespace impl::parser_combinator {

template <typename P>
  requires parser<P>
struct combinator final {

  static_assert(std::same_as<P, strip<P>>);

  using type = type_of<P>;

  [[no_unique_address]] P fn;

  /**
   * @brief Apply the parser to the input `stream`.
   */
  template <typename S = type>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  /**
   * @brief Ignore the result of a parser.
   *
   * This generates a new combinator that behaves like the
   * original but returns `unit` as its value type.
   */
  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(combinate(YETI_FWD(self).fn.skip()))

  /**
   * @brief Ignore the error of a parser.
   *
   * This generates a new combinator that behaves like the
   * original but returns `unit` as its error type.
   */
  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(combinate(YETI_FWD(self).fn.mute()))

  /**
   * @brief The same as `.skip().mute()`.
   */
  [[nodiscard]] constexpr auto drop(this auto &&self)
      YETI_HOF(combinate(YETI_FWD(self).fn.skip().mute()))

  // ===  === //
  // ===  === //
  // ===  === //
};

} // namespace impl::parser_combinator

} // namespace yeti

#endif /* CA19C867_8391_4381_AA78_404CA569C1F7 */
