#ifndef EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7
#define EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7

#include "yeti/core/generics.hpp"
#include <yeti/core/lift.hpp>
#include <yeti/core/parser.hpp>

namespace yeti {

namespace impl::parser_combinator {

template <typename P>
  requires parser<P>
struct combinator;

}

/**
 * @brief The highest level of the parser concept hierarchy.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept combinator =
    parser<P> &&
    specialization_of<strip<P>, impl::parser_combinator::combinator>;

/**
 * @brief An extension of `parser_like` extended to `combinator`.
 */
template <typename P, typename Q>
concept combinator_like =
    parser_fn<Q> &&
    combinator<P, type_of<Q>, parse_value_t<Q>, parse_error_t<Q>>;

// ===  === //
// ===  === //
// ===  === //

template <typename P>
  requires parser_fn<P>
[[nodiscard]] constexpr auto
combinate(P &&p) noexcept(nothrow_storable<P>) -> combinator_like<P> auto {
  return combinate(lift(YETI_FWD(p)));
}

template <typename P>
  requires parser<P>
[[nodiscard]] constexpr auto
combinate(P &&p) noexcept(nothrow_storable<P>) -> combinator_like<P> auto {
  return impl::parser_combinator::combinator<strip<P>>{YETI_FWD(p)};
}

template <typename P>
  requires combinator<P>
[[nodiscard]] constexpr auto combinate(P &&p) noexcept -> P && {
  return YETI_FWD(p);
}

} // namespace yeti

#include "yeti/core/combinate/impl.hpp"

#endif /* EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7 */