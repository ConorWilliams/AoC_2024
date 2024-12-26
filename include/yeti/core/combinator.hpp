#ifndef EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7
#define EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7

#include "yeti/core/combinate/impl.hpp"
#include "yeti/core/generics.hpp"
#include "yeti/core/parser.hpp"

namespace yeti {

/**
 * @brief The highest level of the parser concept hierarchy.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
concept combinator =
    parser<P> && specialization_of<P, impl::parser_combinator::combinator>;

/**
 * @brief An extension of `parser_like` extended to `combinator`.
 */
template <typename P, typename Q>
concept combinator_like =
    parser_fn<Q> && combinator<P, type_of<Q>, parse_value_t<Q>, parse_error_t<Q>>;

// ===  === //
// ===  === //
// ===  === //

// NOTE: `combinate` could include an overload for parser_fn, but it hurts
// compile times due to the constant need to disambiguate between the
// `parser`/`parser_fn`/`combinator` which is quite a hard subsumption
// problem for the compiler to resolve.

/**
 * @brief ...
 */
template <typename P>
  requires parser<P>
[[nodiscard]] constexpr auto
combinate(P &&parser) noexcept(nothrow_storable<P>) -> combinator_like<P> auto {
  return impl::parser_combinator::combinator<strip<P>>{YETI_FWD(parser)};
}

template <typename P>
  requires combinator<P>
[[nodiscard]] constexpr auto combinate(P &&parser) noexcept -> P && {
  return YETI_FWD(parser);
}

} // namespace yeti

#endif /* EFAF34CE_6AFE_403C_AA49_FBDC9BC80BB7 */
