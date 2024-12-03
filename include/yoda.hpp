#ifndef CE2053D2_90CB_4935_B5A1_4A928C8AB29F
#define CE2053D2_90CB_4935_B5A1_4A928C8AB29F

#include <fstream>
#include <sstream>
#include <string>

#include "yoda/combinators.hpp"
#include "yoda/core.hpp"
#include "yoda/parsers.hpp"

namespace yoda {

auto read(std::string const &fname) -> std::string {

  std::ifstream file{fname};

  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + fname);
  }

  return (std::stringstream{} << file.rdbuf()).str();
}

} // namespace yoda

#endif /* CE2053D2_90CB_4935_B5A1_4A928C8AB29F */
