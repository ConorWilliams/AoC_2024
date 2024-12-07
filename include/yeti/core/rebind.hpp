#ifndef DFCDA931_4CDB_417B_A9B9_4D990E349614
#define DFCDA931_4CDB_417B_A9B9_4D990E349614

#include <functional>

#include "yeti/core/parser_fn.hpp"

namespace yeti {

namespace impl::do_rebind {

template <typename...>
struct rebind;

template <typename S, typename T, typename E, typename t, typename e>
struct rebind<result<S, T, E>, t, e> {
  using type = result<S, t, e>;
};

template <typename S, typename T, typename E, typename t>
struct rebind<result<S, T, E>, t, void> {
  using type = result<S, t, E>;
};

template <typename S, typename T, typename E, typename e>
struct rebind<result<S, T, E>, void, e> {
  using type = result<S, T, e>;
};

template <typename S, typename T, typename E>
struct rebind<result<S, T, E>, void, void> {
  using type = result<S, T, E>;
};

} // namespace impl::do_rebind

/**
 * @brief Rebind the result of a parser.
 *
 * I.e. If `P(S) -> result<'S, 'T, 'E>` then `rebind<P, S, T, E>` is
 * `P(S) -> result<`S, 'T if is_void<T> else T, 'E if is_void<E> else E>`.
 */
template <typename P, typename S = void, typename T = void, typename E = void>
  requires parser_fn<P, S> && (typed<P> || not_void<S>)
using rebind = impl::do_rebind::rebind<std::invoke_result_t<P, S>, T, E>::type;

} // namespace yeti

#endif /* DFCDA931_4CDB_417B_A9B9_4D990E349614 */
