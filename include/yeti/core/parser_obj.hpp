#ifndef BE31620B_E9D8_42F5_B265_579BFEA7DE92
#define BE31620B_E9D8_42F5_B265_579BFEA7DE92

#include "yeti/core/parser_fn.hpp"

namespace yeti {

namespace impl {

/**
 * @brief Non recursive bits of the parser concept.
 */
namespace parser_obj_concept {

template <typename P>
concept skippable = requires (P val) { static_cast<P &&>(val).skip(); };

template <typename P>
  requires skippable<P>
using skip_result_t = decltype(std::declval<P>().skip());

template <typename P, typename Q>
concept similar_skippable =
    skippable<P>                                         //
    && skippable<Q>                                      //
    && std::same_as<skip_result_t<P>, skip_result_t<Q>>; //

/**
 * @brief Check that `P.skip() -> _` is valid for all invocations.
 *
 * All the invocations must return the same type.
 */
template <typename P>
concept parser_obj_skippable = skippable<P>                         //
                               && similar_skippable<P, P &>         //
                               && similar_skippable<P, P const &>   //
                               && similar_skippable<P, P &&>        //
                               && similar_skippable<P, P const &&>; //

// ===  === //
// ===  === //
// ===  === //

template <typename P>
concept muteable = requires (P val) { static_cast<P &&>(val).mute(); };

template <typename P>
  requires muteable<P>
using mute_result_t = decltype(std::declval<P>().mute());

template <typename P, typename Q>
concept similar_muteable =
    muteable<P>                                          //
    && muteable<Q>                                       //
    && std::same_as<mute_result_t<P>, mute_result_t<Q>>; //

/**
 * @brief Check that `P.mute() -> _` is valid for all invocations.
 *
 * All the invocations must return the same type.
 */
template <typename P>
concept parser_obj_muteable = muteable<P>                         //
                              && similar_muteable<P, P &>         //
                              && similar_muteable<P, P const &>   //
                              && similar_muteable<P, P &&>        //
                              && similar_muteable<P, P const &&>; //

// ===  === //
// ===  === //
// ===  === //

template <typename P, typename R, typename T, typename E>
concept parser_obj = parser_fn<P, R, T, E>      //
                     && parser_obj_skippable<P> //
                     && parser_obj_muteable<P>; //

} // namespace parser_obj_concept

using impl::parser_obj_concept::parser_obj;

using impl::parser_obj_concept::skip_result_t;

using impl::parser_obj_concept::mute_result_t;

} // namespace impl

} // namespace yeti

#endif /* BE31620B_E9D8_42F5_B265_579BFEA7DE92 */
