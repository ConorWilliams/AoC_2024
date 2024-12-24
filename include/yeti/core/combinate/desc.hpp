#ifndef C4611F7D_9EF7_4FB5_BA9A_6B5B8893FE89
#define C4611F7D_9EF7_4FB5_BA9A_6B5B8893FE89

#include <format>
#include <functional>

#include "yeti/core/blessed.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser.hpp"
#include "yeti/core/rebind.hpp"

namespace yeti::impl::desc {

template <error D, error E>
struct err {

  [[no_unique_address]] D desc;
  [[no_unique_address]] E err;

#define WHAT(x) YETI_FWD(self).x.what()

  [[nodiscard]] constexpr auto what(this auto &&self) {
    if constexpr (std::same_as<E, unit>) {
      return WHAT(desc);
    } else {
      return std::format("{}: '{}'", WHAT(desc), WHAT(err));
    }
  }

#undef WHAT
};

template <parser P, error D>
struct described;

template <typename P, typename D>
[[nodiscard]] constexpr auto describe(P &&p, D &&desc)
    YETI_HOF(described<strip<P>, strip<D>>{YETI_FWD(p), YETI_FWD(desc)})

template <parser P, error D>
struct described {

  static_assert(std::same_as<P, strip<P>>);

  using type = type_of<P>;

  [[no_unique_address]] P fn;
  [[no_unique_address]] D desc;

  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(describe(YETI_FWD(self).fn.skip(), YETI_FWD(self).desc))

  [[nodiscard]] constexpr auto mute(this auto &&self) YETI_HOF(YETI_FWD(self).fn.mute())

  template <typename S = type>
    requires parser<P, S>
  [[nodiscard]] constexpr auto
  operator()(this auto &&self, S &&stream) -> specialization_of<result> auto {

    auto [rest, result] = std::invoke(YETI_FWD(self).fn, YETI_FWD(stream));

    using R = rebind<P, S, void, err<D, parse_error_t<P, S>>>;

    if (result) {
      return R{std::move(rest), std::move(result).value()};
    }

    using Ex = R::expected_type;
    using Er = R::error_type;

    return R{
        std::move(rest),
        Ex{std::unexpect, Er{YETI_FWD(self).desc, std::move(result).error()}},
    };
  }
};

} // namespace yeti::impl::desc

#endif /* C4611F7D_9EF7_4FB5_BA9A_6B5B8893FE89 */
