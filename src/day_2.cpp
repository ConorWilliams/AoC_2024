#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "yoda.hpp"
#include "yoda/combinators.hpp"

// namespace {

// //

// } // namespace

int main() {

  using namespace yoda;

  auto p = rep(lit('8'), 1);

  auto got = yoda::parse(p, "880");

  for (auto &&c : got) {
    std::println("{}", c);
  }

  // std::println("{} {}", a.value.value(), a.rest);

  return 0;
}