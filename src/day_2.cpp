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

#include "parser.hpp"

// namespace {

// //

// } // namespace

int main() {

  auto any = yoda::alt(yoda::any, yoda::eof);

  auto got = yoda::parse(any, "");

  // std::println("{}", got);

  // std::println("{} {}", a.value.value(), a.rest);

  return 0;
}