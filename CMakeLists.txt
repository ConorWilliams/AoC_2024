cmake_minimum_required(VERSION 3.30.5)

project(
  AdventOfCode
  VERSION 1.0
  DESCRIPTION "Advent of Code 2020"
  LANGUAGES CXX
)

# Glob all the source files in the src directory
file(GLOB SOURCES CONFIGURE_DEPENDS src/day_*.cpp)

foreach(SOURCE ${SOURCES})

  cmake_path(GET SOURCE STEM exec_name)

  add_executable(${exec_name} ${SOURCE})

  target_include_directories(${exec_name} PRIVATE include)

  target_compile_features(${exec_name} PRIVATE cxx_std_23)

endforeach()
