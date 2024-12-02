#ifndef BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E
#define BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E

#include <limits>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "yoda/core.hpp"

namespace yoda {

namespace detail {

struct map_impl {
  template <parser P, std::invocable<parser_t<P>> F>
  constexpr auto
  operator()(P p, F f) -> parser_of<std::invoke_result_t<F, parser_t<P>>> auto {
    //
    using R = result<std::invoke_result_t<F, parser_t<P>>>;

    return [p = std::move(p), f = std::move(f)](std::string_view sv) -> R {
      //
      auto r = p(sv);

      if (!r) {
        return {std::unexpected(r.error()), sv};
      }

      return {f(std::move(r).value()), r.rest};
    };
  }
};

} // namespace detail

/**
 * @brief The map combinator.
 */
constexpr detail::map_impl map = {};

namespace detail {

struct seq_impl {
  /**
   * @brief Base case
   */
  template <parser P, parser Q>
  static constexpr auto
  operator()(P p, Q q) -> parser_of<std::tuple<parser_t<P>, parser_t<Q>>> auto {
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
  constexpr static auto operator()(P p, Q q, Ps... ps) -> parser auto {
    return seq_impl{}(p, seq_impl{}(q, ps...));
  }
};

} // namespace detail

/**
 * @brief Sequence multiple combinators (left to right).
 *
 * In the case of more than 2 args: `seq(a, b, c) == seq(a, seq(b, c))`
 */
constexpr detail::seq_impl seq = {};

namespace detail {

template <class... Args, class R = std::unexpected<std::string>>
constexpr auto err(std::format_string<Args...> fmt, Args &&...args) -> R {
  return std::unexpected(std::format(fmt, std::forward<Args>(args)...));
}

template <typename L, typename R> struct alt_result {
  using type = std::variant<L, R>;
};

template <typename T> struct alt_result<T, T> {
  using type = T;
};

template <typename P, typename Q>
using alt_variant_type = alt_result<parser_t<P>, parser_t<Q>>::type;

struct alt_impl {
  template <parser P, parser Q>
  static constexpr auto
  operator()(P p, Q q) -> parser_of<alt_variant_type<P, Q>> auto {
    //
    using R = result<alt_variant_type<P, Q>>;

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

      return {err(fmt, lhs.error(), rhs.error()), sv};
    };
  }

  template <parser P, parser Q, parser... Ps>
  constexpr static auto operator()(P p, Q q, Ps... ps) -> parser auto {
    return alt_impl{}(p, alt_impl{}(q, ps...));
  }
};

} // namespace detail

/**
 * @brief Alternative multiple combinator (left to right).
 *
 * In the case of more than 2 args: `alt(a, b, c) == alt(a, alt(b, c))`
 */
constexpr detail::alt_impl alt = {};

namespace detail {

template <parser P> using rep_vector_t = std::vector<parser_t<P>>;

struct rep_impl {

  // clang-format off

  template <parser P>
  constexpr auto 
  operator()(P p, std::size_t min, std::size_t max) -> parser_of<rep_vector_t<P>> auto {

    // clang-format on

    using R = result<std::vector<parser_t<P>>>;

    constexpr std::string_view fmt = "Expected at least {} repetitions, got {}";

    return [=, p = std::move(p)](std::string_view sv) -> R {
      //
      std::vector<parser_t<P>> acc;

      std::string_view rest = sv;

      for (std::size_t i = 0; i < max; ++i) {
        if (auto r = p(rest)) {
          rest = r.rest;
          acc.push_back(std::move(r).value());
        } else {
          break;
        }
      }

      if (acc.size() < min) {
        return {err(fmt, min, acc.size()), rest};
      }

      return {std::move(acc), rest};
    };
  }
};

} // namespace detail

/**
 * @brief Kleene star combinator.
 */
constexpr auto star = [](parser auto p) -> parser auto {
  return detail::rep_impl{}(p, 0, std::numeric_limits<std::size_t>::max());
};

/**
 * @brief Kleene plus combinator.
 */
constexpr auto plus = [](parser auto p) -> parser auto {
  return detail::rep_impl{}(p, 1, std::numeric_limits<std::size_t>::max());
};

/**
 * @brief Repetition combinator.
 */
constexpr auto rep = [](parser auto p, std::size_t n) -> parser auto {
  return detail::rep_impl{}(p, n, n);
};

} // namespace yoda

#endif /* BD6C0AE4_ED25_4BA6_9686_903FFEBDED6E */
