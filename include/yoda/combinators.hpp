#ifndef BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E
#define BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E

#include <tuple>

#include "yoda/core.hpp"

namespace yoda {

namespace detail {

struct seq_impl {
  /**
   * @brief Base case
   */
  template <parser P, parser Q> static constexpr auto operator()(P p, Q q) {
    //
    using Tup = std::tuple<parser_t<P>, parser_t<Q>>;

    using R = result<Tup>;

    return [p = std::move(p), q = std::move(q)](std::string_view sv) -> R {
      //
      auto lhs = p(sv);

      if (!lhs) {
        return {std::unexpected(lhs.error()), sv};
      }

      auto rhs = q(lhs.rest);

      if (!rhs) {
        return {std::unexpected(rhs.error()), lhs.rest};
      }

      return {Tup{std::move(lhs).value(), std::move(rhs).value()}, rhs.rest};
    };
  }

  /**
   * @brief Recursive case
   */
  template <parser P, parser Q, parser... Ps>
  constexpr static auto operator()(P p, Q q, Ps... ps) {
    return seq_impl{}(seq_impl{}(p, q), ps...);
  }
};

} // namespace detail

/**
 * @brief Sequence multiple combinator (left to right).
 */
constexpr detail::seq_impl seq = {};

namespace detail {

struct alt_impl {
  template <parser P, parser Q> constexpr auto operator()(P p, Q q) {
    //
    using RQ = parser_t<Q>;
    using RP = parser_t<P>;

    constexpr bool same = std::is_same_v<RP, RQ>;

    using R = result<std::conditional_t<same, RP, std::variant<RP, RQ>>>;

    return [p = std::move(p), q = std::move(q)](std::string_view sv) -> R {
      //
      auto lhs = p(sv);

      if (lhs) {
        return {std::move(lhs).value(), lhs.rest};
      }

      auto rhs = q(sv);

      if (rhs) {
        return {std::move(rhs).value(), rhs.rest};
      }

      constexpr std::string_view fmt = "Both parsers failed with:\n\t{}\n\t{}";

      return {unexpected(std::format(fmt, lhs.error(), rhs.error())), sv};
    };
  }

  template <parser P, parser Q, parser... Ps>
  constexpr auto operator()(P p, Q q, Ps... ps) {
    return alt_impl{}(alt_impl{}(p, q), ps...);
  }
};

} // namespace detail

/**
 * @brief Alternative multiple combinator (left to right).
 */
constexpr detail::alt_impl alt = {};

/**
 * @brief Kleene star combinator.
 */
constexpr auto star = []<parser P>(P p) {
  //
  using RP = parser_t<P>;

  return [p = std::move(p)](std::string_view sv) -> result<std::vector<RP>> {
    //
    std::vector<RP> acc;

    std::string_view rest = sv;

    for (;;) {
      if (auto r = p(rest)) {
        rest = r.rest;
        acc.push_back(std::move(r).value());
      } else {
        break;
      }
    }

    return {std::move(acc), rest};
  };
};

} // namespace yoda

#endif /* BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E */
