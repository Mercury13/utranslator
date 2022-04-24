//
// Reichwein.IT Unicode Library
//
// Functions for conversion between UTF and ISO encodings
//

#pragma once

#include "unicode/endian.h"
#include "unicode/iso.h"
#include "unicode/optimization.h"
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

 // First variant of convert(): Specification of encodings explicitly
 //
 // e.g.
 // unicode::UTF_8
 // unicode::UTF_16
 // unicode::UTF_32
 // unicode::ISO_8859_1
 // unicode::ISO_8859_15
 //
 // see also utf.h and iso.h
 //
 // From and To are Encodings
 //
 // throws std::invalid_argument on conversion error
 template<typename From, typename To, std::enable_if_t<is_encoding_v<From> && is_encoding_v<To>, bool> = true>
 typename To::string_type convert(const typename From::string_type& s)
 {
  // At compile time, decide which optimization to use, with fallback to
  // iterating with std::copy()

  // if input type == output type, only validate and return input, if appropriate
  if constexpr(sizeof(typename From::value_type) == sizeof(typename To::value_type) &&
               is_utf_encoding_v<From> && is_utf_encoding_v<To>) {
   if (validate_utf<typename From::value_type>(s)) {
    return s;
   } else {
    throw std::invalid_argument("Invalid UTF input");
   }
  } else if constexpr(accu_size == 8 && is_little_endian() && is_utf_8_v<typename From::value_type> &&
                      is_utf_encoding_v<From> && is_utf_encoding_v<To>) { // endian specific optimization
   return convert_optimized_utf<From, To>(s);
  } else if constexpr(accu_size == 4 || accu_size == 8) { // accu size specific optimization with speedup for 7bit input
   return convert_optimized<From, To>(s);
  } else {
   typename To::string_type result;
   std::copy(From::begin(s), From::end(s), To::back_inserter(result));
   return result;
  }
 }

 // Second variant of convert(): Specification of encodings via character type
 //
 // see also type_traits.h for is_char
 //
 // From and To are from: utf8_t (i.e. char or char8_t (C++20)), char16_t and char32_t, char, wchar_t, uint8_t, uint16_t, uint32_t
 //
 // throws std::invalid_argument on conversion error
 template<typename From, typename To,
  typename FromContainer=std::basic_string<From>,
  typename ToContainer=std::basic_string<To>,
  std::enable_if_t<is_char_v<From> && is_char_v<To>, bool> = true>
 ToContainer convert(const FromContainer& s)
 {
  typedef UTF<utf_iterator<From>, utf_back_insert_iterator<To>> UTF_Trait;

  ToContainer result;

  std::copy(UTF_Trait::begin(s), UTF_Trait::end(s), UTF_Trait::back_inserter(result));

  return result;
 }

 // Third variant of convert(): Specification of encodings via container type
 //
 // see also type_traits.h for is_container
 //
 // From and To are containers
 //
 // throws std::invalid_argument on conversion error
 template<typename FromContainer, typename ToContainer,
  std::enable_if_t<is_container_v<FromContainer> && is_container_v<ToContainer>, bool> = true
 >
 ToContainer convert(const FromContainer& s)
 {
  typedef UTF<utf_iterator<typename FromContainer::value_type, FromContainer>, utf_back_insert_iterator<typename ToContainer::value_type, ToContainer>> UTF_Trait;
  
  ToContainer result;

  std::copy(UTF_Trait::begin(s), UTF_Trait::end(s), UTF_Trait::back_inserter(result));

  return result;
 }

} // namespace unicode

