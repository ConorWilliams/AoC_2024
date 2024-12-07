#ifndef CA19C867_8391_4381_AA78_404CA569C1F7
#define CA19C867_8391_4381_AA78_404CA569C1F7

#include "yeti/core/generics.hpp"
#include "yeti/core/typed.hpp"
#include <yeti/core/lift.hpp>
#include <yeti/core/parser.hpp>

namespace yeti {

namespace impl::parser_combinator {

template <typename P>
  requires parser<P>
struct combinator final {

  using type = type_of<P>;

  [[no_unique_address]] P fn;

  template <typename S = type>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(YETI_FWD(self).fn.skip())

  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(YETI_FWD(self).fn.skip())

  // ===  === //
  // ===  === //
  // ===  === //
};

} // namespace impl::parser_combinator

} // namespace yeti

#endif /* CA19C867_8391_4381_AA78_404CA569C1F7 */
