//
// Reichwein.IT Unicode Library
//
// Predicates for Unicode characters
//

#pragma once

namespace unicode {

 // Detection of a valid Unicode code point value. Independent of encoding.
 //
 // Note: This doesn't tell if the specified value is actually allocated in an
 // existing Unicode version, but rather just detects if the value is inside
 // allocatable range.
 // 
 // bits_to_compare: limit bits to consider even further than defined by T
 // T: usually, char32_t, uint32_t etc.
 template<size_t bits_to_compare = 32, typename T>
 static inline bool is_valid_unicode(const T& value) noexcept
 {
  if constexpr(sizeof(T) == 1 || bits_to_compare <= 15)
   return true;
  else if constexpr(sizeof(T) == 2 || bits_to_compare <= 20)
   //return value <= 0xD7FF || value >= 0xE000;
   return (value & 0xF800) != 0xD800;
  else
   //return (value & 0xFFFFF800) != 0x0000D800 && (value >> 16) <= 0x10;
   return value <= 0xD7FF || (value >= 0xE000 && value <= 0x10FFFF);
 }

} // namespace unicode

