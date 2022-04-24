//
// Reichwein.IT Unicode Library
//
// Endian handling functions
//
// In C++17, endian support is not yet available.
//

#pragma once

#if __cplusplus >= 202002L
#include <bit>
#endif
#include <cstdint>

namespace unicode::detail {

#if __cplusplus >= 202002L
 consteval
#else
 constexpr uint16_t endian_value{0x0102};
 constexpr uint8_t endian_value_1st_byte{(const uint8_t&)endian_value};

 constexpr
#endif
 bool is_little_endian()
 {
#if __cplusplus >= 202002L
  return std::endian::native == std::endian::little;
#else
  return endian_value_1st_byte == 0x02;
#endif
 }

} // namespace unicode::detail
