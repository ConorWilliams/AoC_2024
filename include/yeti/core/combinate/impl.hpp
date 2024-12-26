#ifndef CA19C867_8391_4381_AA78_404CA569C1F7
#define CA19C867_8391_4381_AA78_404CA569C1F7

#include <concepts>
#include <functional>

#include "yeti/core/generics.hpp"
#include "yeti/core/parser.hpp"
#include "yeti/core/typed.hpp"

#include "yeti/core/combinate/desc.hpp"
#include "yeti/core/combinate/typed.hpp"

namespace yeti {

namespace impl::parser_combinator {

template <typename P>
  requires parser<P>
struct combinator;

template <typename P>
[[nodiscard]] constexpr auto
recombinate(P &&parser) noexcept(nothrow_storable<P>) -> combinator<strip<P>> {
  return {YETI_FWD(parser)};
}

template <specialization_of<combinator> P>
[[nodiscard]] constexpr auto recombinate(P &&parser) noexcept -> P && {
  return YETI_FWD(parser);
}

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
    requires parser<P, S>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  /**
   * @brief Ignore the result of a parser.
   *
   * This generates a new combinator that behaves like the
   * original but returns `unit` as its value type.
   */
  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(recombinate(YETI_FWD(self).fn.skip()))

  /**
   * @brief Ignore the error of a parser.
   *
   * This generates a new combinator that behaves like the
   * original but returns `unit` as its error type.
   */
  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(recombinate(YETI_FWD(self).fn.mute()))

  /**
   * @brief The same as `.skip().mute()`.
   */
  [[nodiscard]] constexpr auto drop(this auto &&self)
      YETI_HOF(recombinate(YETI_FWD(self).fn.skip().mute()))

  // ===  === //
  // ===  === //
  // ===  === //

  template <storable E>
    requires error<strip<E>>
  [[nodiscard]] constexpr auto desc(this auto &&self, E &&err)
      YETI_HOF(recombinate(desc::describe(YETI_FWD(self).fn, YETI_FWD(err))))

  // TODO: Describe with a static string

  // ===  === //
  // ===  === //
  // ===  === //

  template <typename Type>
    requires pure_void<type> && std::same_as<Type, strip<Type>>
  [[nodiscard]] constexpr auto typed(this auto &&self)
      YETI_HOF(recombinate(typed::with_type<Type>(YETI_FWD(self).fn)))
};

} // namespace impl::parser_combinator

} // namespace yeti

#endif /* CA19C867_8391_4381_AA78_404CA569C1F7 */
