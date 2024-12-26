#ifndef jh383khfkjh8y33lajnc34pfucy3jkhf
#define jh383khfkjh8y33lajnc34pfucy3jkhf

#include <functional>

#include "yeti/core/generics.hpp"
#include "yeti/core/parser.hpp"

namespace yeti::impl::typed {

template <typename P, typename T>
struct typed;

template <typename T, typename P>
constexpr auto with_type(P &&parser) -> typed<strip<P>, T> {
  return {YETI_FWD(parser)};
}

template <typename P, typename T>
struct typed {

  static_assert(std::same_as<P, strip<P>>);
  static_assert(std::same_as<T, strip<T>>);

  [[no_unique_address]] P fn;

  using type = T;

  template <typename S = type>
    requires parser<P, S> && std::same_as<T, strip<S>>
  [[nodiscard]] constexpr auto operator()(this auto &&self, S &&stream)
      YETI_HOF(std::invoke(YETI_FWD(self).fn, YETI_FWD(stream)))

  [[nodiscard]] constexpr auto skip(this auto &&self)
      YETI_HOF(with_type<T>(YETI_FWD(self).fn.skip()))

  [[nodiscard]] constexpr auto mute(this auto &&self)
      YETI_HOF(with_type<T>(YETI_FWD(self).fn.mute()))
};

} // namespace yeti::impl::typed

#endif
