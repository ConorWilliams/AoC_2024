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

  auto p = yoda::seq(yoda::any, yoda::any, yoda::eof);

  auto got = yoda::parse(p, "880");

  // for (auto &&c : got) {
  //   std::println("{}", c);
  // }

  // std::println("{} {}", a.value.value(), a.rest);

  return 0;
}