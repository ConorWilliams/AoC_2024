#include <algorithm>
#include <fstream>
#include <map>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "yoda.hpp"

namespace {

struct Parsed {
  std::vector<int> lhs;
  std::vector<int> rhs;
};

auto parse(std::string const &fname) -> Parsed {

  using namespace yoda;

  std::string file = yoda::read(fname);

  std::println("{}", file);

  parser auto p = number<int> //
                      .seq_left(ws.plus())
                      .seq(number<int>)
                      .seq_left(eol)
                      .star();

  // parser auto line = seq(number<int>, plus(number<int>), );

  // int a = 0;
  // int b = 0;

  // while (file >> a >> b) {
  //   parsed.lhs.push_back(a);
  //   parsed.rhs.push_back(b);
  // }

  // return parsed;
}

namespace views = std::ranges::views;

} // namespace

int main() try {

  // Part I

  Parsed p = parse("./inputs/day_1.txt");

  std::ranges::sort(p.lhs);
  std::ranges::sort(p.rhs);

  auto abs_diff = [](int a, int b) {
    return std::abs(a - b);
  };

  auto all_diffs = views::zip_transform(abs_diff, p.lhs, p.rhs);

  auto delta = std::ranges::fold_left(all_diffs, 0, std::plus{});

  std::println("Part  I: {}", delta);

  if (delta != 1879048) {
    throw std::runtime_error("Day 1 regressed");
  }

  // Part II

  std::map<int, int> counts{};

  for (auto x : p.rhs) {
    counts[x] += 1;
  }

  auto sims = p.lhs | views::transform([&counts](int x) {
                return x * counts[x];
              });

  auto sim = std::ranges::fold_left(sims, 0, std::plus{});

  std::println("Part II: {}", sim);

  if (sim != 21024792) {
    throw std::runtime_error("Day 1 regressed");
  }

  return 0;

} catch (std::exception const &e) {
  std::println("Error: {}", e.what());
  return 1;
}