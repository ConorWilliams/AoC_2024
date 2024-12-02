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

// namespace {

// //

// } // namespace

int main() {

  auto p = yoda::star(yoda::any);

  auto got = yoda::parse(p, "890");

  for (auto &&c : got) {
    std::println("{}", c);
  }

  // std::println("{} {}", a.value.value(), a.rest);

  return 0;
}