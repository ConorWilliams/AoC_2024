

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

  //   using type = std::string_view;

  auto static
  operator()(std::string_view input) -> result<std::string_view, unit, E> {
    return {{}, input};
  }

  static auto skip() -> noop { return {}; }

  static auto mute() -> noop<unit> { return {}; }
};

static_assert(parser<noop<>>);

static_assert(parser<noop<>, std::string_view>);

static_assert(parser<noop<>, std::string_view, unit>);

// This is expected as we cannot know T until we know R
static_assert(!parser<noop<>, void, unit>);

static_assert(parser<noop<>, std::string_view, unit, never>);

static_assert(parser<noop<>, std::string_view, void, never>);

struct tt : noop<> {
  using type = std::string_view;
};

static_assert(parser<tt, void, unit>);

// static_assert(parser_of<bare, unit>);

} // namespace

int main() { return 0; }