#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::optional<int> to_int(std::string_view x) {
  int out;

  auto result = std::from_chars(x.data(), x.data() + x.size(), out);

  if (result.ec == std::errc::invalid_argument ||
      result.ec == std::errc::result_out_of_range) {
    return std::nullopt;
  }

  return out;
}

std::vector<int> split(std::string_view line) {

  std::vector<int> list;

  auto start = line.begin();
  auto end = line.begin();

  while (end != line.end()) {
    if (*end == ' ') {
      if (auto value = to_int({&*start, static_cast<size_t>(end - start)})) {
        list.push_back(*value);
      }

      start = end + 1;
    }

    end++;
  }

  if (auto value = to_int({&*start, static_cast<size_t>(end - start)})) {
    list.push_back(*value);
  }

  return list;
}

struct Parsed {
  std::vector<std::vector<int>> reports;
};

auto parse(std::string const &fname) -> Parsed {

  // Open the file for reading
  std::ifstream file{fname};

  if (!file.is_open()) {
    throw std::runtime_error("Could not open file");
  }

  Parsed parsed;

  for (std::string line; std::getline(file, line);) {

    // parsed.reports.push_back(std::vector<int>(t.begin(), t.end()));
  }

  return parsed;
}

namespace views = std::ranges::views;

} // namespace

int main() {

  std::cout << "Hello, World!" << std::endl;

  Parsed data = parse("./inputs/day_2.txt");

  return 0;
}