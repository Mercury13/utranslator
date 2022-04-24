//
// Reichwein.IT Unicode Library
//
// Optimized conversion functions for UTF input and output
//

#pragma once

#include "unicode/endian.h"
#include "unicode/iso.h"
#include "unicode/predicate.h"
#include "unicode/types.h"
#include "unicode/type_traits.h"
#include "unicode/utf.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace unicode {

 // Helper function: Item distance of specified iterators
 // std::distance doesn't work here: it is based on "output" distance of iterators
 template<class Iterator>
 inline size_t input_distance(const Iterator& it1, const Iterator& it2)
 {
  return it2 - it1;
 }
 
 // Helper function: Distance of specified iterator content data in bytes
 template<class Iterator>
 inline size_t input_distance_bytes(const Iterator& it1, const Iterator& it2)
 {
  return input_distance(it1, it2) * sizeof(typename Iterator::value_type);
 }

 // Optimizations following:
 static const size_t accu_size {sizeof(size_t)};

 template<int value_size>
 struct ConvertInputOptimizer {};

 template<> struct ConvertInputOptimizer<1>
 {
  static const uint32_t ascii_mask { 0x80808080 };
 };
 
 template<> struct ConvertInputOptimizer<2>
 {
  static const uint32_t ascii_mask { 0xFF80FF80 };
 };
 
 template<> struct ConvertInputOptimizer<4>
 {
  static const uint32_t ascii_mask { 0xFFFFFF80 };
 };

 template<int AccuSize, class ConvertInputOptimizer>
 struct ArchitectureOptimizer {};

 // On 32 bit architecture, calculate with 32 bit accumulator value
 // (hoping the compiler will put it into a 32 bit register)
 template<class ConvertInputOptimizer>
 struct ArchitectureOptimizer<4, ConvertInputOptimizer>
 {
  typedef ConvertInputOptimizer input_optimizer;
  typedef uint32_t accu_type;
  static const accu_type addr_mask {accu_size - 1};
  static const accu_type ascii_mask { (accu_type)input_optimizer::ascii_mask };
  static const accu_type ascii_value { 0ULL };
  
  template<typename input_value_type, class output_string_type>
  inline static void append(const input_value_type* addr, output_string_type& s)
  {
   if constexpr(sizeof(input_value_type) == sizeof(typename output_string_type::value_type)) {
    s.append(reinterpret_cast<const typename output_string_type::value_type*>(addr), accu_size / sizeof(input_value_type));
   } else if constexpr(is_utf_8_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0]),
              static_cast<typename output_string_type::value_type>(addr[1]),
              static_cast<typename output_string_type::value_type>(addr[2]),
              static_cast<typename output_string_type::value_type>(addr[3])});
   } else if constexpr(is_utf_16_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0]),
              static_cast<typename output_string_type::value_type>(addr[1])});
   } else if constexpr(is_utf_32_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0])});
   }
  }
 };

 // On 64 bit architecture, calculate with 64 bit accumulator value
 // (hoping the compiler will put it into a 64 bit register)
 template<class ConvertInputOptimizer>
 struct ArchitectureOptimizer<8, ConvertInputOptimizer>
 {
  typedef ConvertInputOptimizer input_optimizer;
  typedef uint64_t accu_type;
  static const accu_type addr_mask {accu_size - 1};
  static const accu_type ascii_mask { ((accu_type)input_optimizer::ascii_mask) << 32 | (accu_type)input_optimizer::ascii_mask };
  static const accu_type ascii_value { 0ULL };
  
  template<typename input_value_type, class output_string_type>
  inline static void append(const input_value_type* addr, output_string_type& s)
  {
   if constexpr(sizeof(input_value_type) == sizeof(typename output_string_type::value_type)) {
    s.append(reinterpret_cast<const typename output_string_type::value_type*>(addr), accu_size / sizeof(input_value_type));
   } else if constexpr(is_utf_8_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0]),
              static_cast<typename output_string_type::value_type>(addr[1]),
              static_cast<typename output_string_type::value_type>(addr[2]),
              static_cast<typename output_string_type::value_type>(addr[3]),
              static_cast<typename output_string_type::value_type>(addr[4]),
              static_cast<typename output_string_type::value_type>(addr[5]),
              static_cast<typename output_string_type::value_type>(addr[6]),
              static_cast<typename output_string_type::value_type>(addr[7])});
   } else if constexpr(is_utf_16_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0]),
              static_cast<typename output_string_type::value_type>(addr[1]),
              static_cast<typename output_string_type::value_type>(addr[2]),
              static_cast<typename output_string_type::value_type>(addr[3])});
   } else if constexpr(is_utf_32_v<input_value_type>) {
    s.append({static_cast<typename output_string_type::value_type>(addr[0]),
              static_cast<typename output_string_type::value_type>(addr[1])});
   }
  }

 }; // class ArchitectureOptimizer

 // Optimize for the case of all ASCII (7-bit) data in a accu size row
 // From and To are Encodings
 template<typename From, typename To, std::enable_if_t<is_encoding_v<From> && is_encoding_v<To>, bool> = true>
 typename To::string_type convert_optimized(const typename From::string_type& s)
 {
  typename To::string_type result;
  typedef ConvertInputOptimizer<sizeof(typename From::value_type)> input_optimizer;
  typedef ArchitectureOptimizer<accu_size, input_optimizer> arch_optimizer;

  auto begin{From::begin(s)};
  auto end{From::end(s)};
  auto back_inserter{To::back_inserter(result)};
  auto addr{reinterpret_cast<const typename arch_optimizer::accu_type*>(&s.data()[s.size() - input_distance(begin, end)])};
  while (input_distance_bytes(begin, end) >= accu_size) {
   if (((uintptr_t)(void*)addr & arch_optimizer::addr_mask) == 0) {
    while (input_distance_bytes(begin, end) >= accu_size) {
     typename arch_optimizer::accu_type data{*addr};
     if ((data & arch_optimizer::ascii_mask) == arch_optimizer::ascii_value)
#if __cplusplus >= 202002L
     [[likely]]
#endif
     {
      arch_optimizer::template append(reinterpret_cast<const typename From::value_type*>(addr), result);
      begin += accu_size / sizeof(typename From::value_type);
      ++addr;
     } else {
      // just advance one code unit for now and break to trigger unoptimized
      // version until next accu boundary
      back_inserter = *begin;
      ++begin;
      break;
     }
    }
   }

   // keep up after unaligned Non-ASCII code points
   while (begin != end && (uintptr_t)(void*)(addr = reinterpret_cast<const typename arch_optimizer::accu_type*>(&s.data()[s.size() - input_distance(begin, end)])) & arch_optimizer::addr_mask) {
    back_inserter = *begin;
    ++begin;
   }
  }

  // remainder < 8 bytes   
  while (begin != end) {
   back_inserter = *begin;
   ++begin;
  }

  return result;
 }

 template<size_t bits_to_compare = 32, typename To, typename std::enable_if_t<is_utf_8_v<To>, bool> = true>
 inline void append_utf(std::basic_string<To>& result, const char32_t& value)
 {
  using From = char32_t;
  if (bits_to_compare <= 7 || value < 0x80) { // 1 byte
   result.push_back(static_cast<To>(value));
  } else if (bits_to_compare <= 11 || value < 0x800) { // 2 bytes
   result.append({utf8_byte_n_of_m<0,2,From,To>(value), utf8_byte_n_of_m<1,2,From,To>(value)});
  } else if (bits_to_compare <= 16 || value < 0x10000) { // 3 bytes
   result.append({utf8_byte_n_of_m<0,3,From,To>(value), utf8_byte_n_of_m<1,3,From,To>(value), utf8_byte_n_of_m<2,3,From,To>(value)});
  } else { // 4 bytes
   // expect value to be already valid Unicode values
   result.append({utf8_byte_n_of_m<0,4,From,To>(value), utf8_byte_n_of_m<1,4,From,To>(value), utf8_byte_n_of_m<2,4,From,To>(value), utf8_byte_n_of_m<3,4,From,To>(value)});
  }
 }

 template<size_t bits_to_compare = 32, typename To, typename std::enable_if_t<is_utf_16_v<To>, bool> = true>
 inline void append_utf(std::basic_string<To>& result, const char32_t& value)
 {
  if (bits_to_compare <= 16 || value <= 0xFFFF) { // expect value to be already valid Unicode values
   result.push_back(static_cast<To>(value));
  } else {
   char32_t value_reduced{value - 0x10000};
   result.append({static_cast<To>((value_reduced >> 10) + 0xD800), static_cast<To>((value_reduced & 0x3FF) + 0xDC00)});
  }
 }

 template<size_t bits_to_compare = 32, typename To, typename std::enable_if_t<is_utf_32_v<To>, bool> = true>
 inline void append_utf(std::basic_string<To>& result, const char32_t& value)
 {
  // expect value to be already valid Unicode values (checked in input iterator)
  result.push_back(static_cast<To>(value));
 }

 // Little Endian optimized version for UTF-8
 // In block_mode, at least 4 bytes are in accu. On first call, even 8.
 // otherwise, at least one code unit is in accu
 template<typename From, typename To, bool block_mode = true, typename std::enable_if_t<is_utf_8_v<From>, bool> = true>
 inline static void append_accu(std::basic_string<To>& result, uint64_t& accu, int& bytes_in_accu)
 {
  if (block_mode && bytes_in_accu == 8 && (accu & 0x8080808080808080) == 0)
#if __cplusplus >= 202002L
  [[likely]]
#endif
  {
   result.append({
                 static_cast<To>(accu & 0x7F),
                 static_cast<To>((accu >> 8) & 0x7F),
                 static_cast<To>((accu >> 16) & 0x7F),
                 static_cast<To>((accu >> 24) & 0x7F),
                 static_cast<To>((accu >> 32) & 0x7F),
                 static_cast<To>((accu >> 40) & 0x7F),
                 static_cast<To>((accu >> 48) & 0x7F),
                 static_cast<To>((accu >> 56) & 0x7F),
                 });
   accu = 0;
   bytes_in_accu = 0;
  } else if ((accu & 0x80) == 0) { // 1 byte sequence
   append_utf<7>(result, static_cast<char32_t>(accu & 0x7F));
   accu >>= 8;
   bytes_in_accu -= 1;
  } else if ((block_mode || bytes_in_accu >= 2) && (accu & 0xC0E0) == 0x80C0) { // 2 byte sequence
   char32_t value {static_cast<char32_t>(((accu & 0x1F) << 6) | ((accu >> 8) & 0x3f))};
   accu >>= 16;
   bytes_in_accu -= 2;
   append_utf<11>(result, value); // 11 bit Unicode values are always valid Unicode
  } else if ((block_mode || bytes_in_accu >= 3) && (accu & 0xC0C0F0) == 0x8080E0) { // 3 byte sequence
   char32_t value {static_cast<char32_t>(((accu & 0x0F) << 12) | ((accu >> 2) & 0x0FC0) | ((accu >> 16) & 0x3f))};
   accu >>= 24;
   bytes_in_accu -= 3;
   if (is_valid_unicode<16>(value))
    append_utf<16>(result, value);
   else
#if __cplusplus >= 202002L
    [[unlikely]]
#endif
    throw std::invalid_argument("Invalid Unicode character in 3 byte UTF-8 sequence");
  } else if ((block_mode || bytes_in_accu >= 4) && (accu & 0xC0C0C0F8) == 0x808080F0) { // 4 byte sequence
   char32_t value {static_cast<char32_t>(((accu & 0x07) << 18) | ((accu << 4) & 0x3f000) | ((accu >> 10) & 0xFC0) | ((accu >> 24) & 0x3f))};
   accu >>= 32;
   bytes_in_accu -= 4;
   if (is_valid_unicode<21>(value))
    append_utf(result, value);
   else
#if __cplusplus >= 202002L
    [[unlikely]]
#endif
    throw std::invalid_argument("Invalid Unicode character in 4 byte UTF-8 sequence");
  } else
#if __cplusplus >= 202002L
   [[unlikely]]
#endif
   throw std::invalid_argument("Invalid UTF-8 byte sequence");
 }

 // Little Endian optimized version
 template<typename From, typename To, std::enable_if_t<is_encoding_v<From> && is_encoding_v<To>, bool> = true>
 typename To::string_type convert_optimized_utf(const typename From::string_type& s)
 {
  typename To::string_type result;
  uint64_t accu{};
  int bytes_in_accu{};

  size_t s_index{};
  size_t s_size{s.size()};
  while (s_index + 8 / sizeof(typename From::value_type) <= s_size) {
   // read input
   // assume: bytes_in_accu < 8
   accu |= (*reinterpret_cast<const uint64_t*>(&(s.data()[s_index]))) << (bytes_in_accu * 8);
   s_index += (8 - bytes_in_accu) / sizeof(typename From::value_type);
   bytes_in_accu = 8;

   while (bytes_in_accu >= 4) {
    append_accu<typename From::value_type, typename To::value_type, true>(result, accu, bytes_in_accu);
   }
  }

  // 0..3 bytes left in accu
  // 0..7 bytes left in s

  while (s_index < s_size || bytes_in_accu > 0) {
   while (s_index < s_size && bytes_in_accu < 8) {
    accu |= static_cast<uint64_t>(*reinterpret_cast<const uint8_t*>(&(s.data()[s_index]))) << (bytes_in_accu * 8);
    ++s_index;
    bytes_in_accu += sizeof(typename From::value_type);
   }

   append_accu<typename From::value_type, typename To::value_type, false>(result, accu, bytes_in_accu);
  }
  return result;
 }

} // namespace unicode

