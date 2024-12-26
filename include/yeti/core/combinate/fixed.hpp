#pragma once

#include <cstddef>
#include <utility>

namespace yeti {

template <typename T>
struct fixed_value {
  using type = T;
  constexpr fixed_value(type v) noexcept : data(v) {}
  T data;
};

template <typename T, std::size_t Extent>
struct fixed_value<T[Extent]> {
  using type = T[Extent];
  constexpr fixed_value(T (&arr)[Extent]) noexcept
      : fixed_value(arr, std::make_index_sequence<Extent>()) {}
  T data[Extent];

 private:
  template <std::size_t... Idx>
  constexpr fixed_value(T (&arr)[Extent], std::index_sequence<Idx...>) noexcept
      : data{arr[Idx]...} {}
};

template <typename T, std::size_t Extent>
fixed_value(T (&)[Extent]) -> fixed_value<T[Extent]>;

template <typename T>
fixed_value(T) -> fixed_value<T>;

} // namespace yeti
