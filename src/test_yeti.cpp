

#include "yeti/core.hpp"

namespace {

using namespace yeti;

static_assert(error<unit>);

/**
 * noop<err>
 *   skip() -> noop<err>
 *   mute() -> noop<unit>
 *     skip() -> noop<unit>
 *     mute() -> noop<unit>
 */

template <typename E = never>
struct noop {

  auto static
  operator()(std::string_view input) -> result<std::string_view, unit, E> {
    return {{}, input};
  }

  static auto skip() -> noop { return {}; }

  static auto mute() -> noop<unit> { return {}; }
};

struct bare {
  auto static
  operator()(std::string_view input) -> result<std::string_view, unit, unit> {
    return {{}, input};
  }

  static auto skip() -> bare { return {}; }

  static auto mute() {}
};

static_assert(std::same_as<impl::parser::parse_value_t<bare>, void>);

static_assert(
    std::same_as<impl::parser::parse_value_t<bare, std::string_view>, unit>);

static_assert(impl::parser::core_parser<bare>);

static_assert(impl::parser::core_parser<bare, std::string_view, unit>);

static_assert(impl::parser::parser<noop<>, std::string_view>);
// static_assert(std::same_as<parse_error_t<bare>, unit>);

// static_assert(parser_of<bare, unit>);

} // namespace

int main() {

  std::string_view sv = "hello";

  bare b;

  //   static_assert(std::invocable<bare const, std::string_view>);

  b(sv);
  //   static_cast<bare const &>(b)(sv);

  return 0;
}