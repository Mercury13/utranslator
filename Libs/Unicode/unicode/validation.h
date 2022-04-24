//
// Reichwein.IT Unicode Library
//
// Functions for validation of UTF (Unicode Transformation Format) encodings
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

 // First variant of is_valid_utf(): Specification of encoding explicitly
 //
 // e.g.
 // unicode::UTF_8
 // unicode::UTF_16
 // unicode::UTF_32
 //
 // see also type_traits.h and utf.h
 template<typename Encoding, std::enable_if_t<is_encoding_v<Encoding>, bool> = true>
 bool is_valid_utf(const typename Encoding::string_type& s)
 {
  return validate_utf<typename Encoding::value_type>(s);
 }

 // Second variant of is_valid_utf(): Specification of encoding via character type
 //
 // see also type_traits.h for is_char
 template<typename T,
  typename Container=std::basic_string<T>,
  std::enable_if_t<is_char_v<T>, bool> = true>
 bool is_valid_utf(const Container& s)
 {
  typedef UTF<utf_iterator<T>, utf_back_insert_iterator<T>> UTF_Trait;
  
  try {
   std::for_each(UTF_Trait::begin(s), UTF_Trait::end(s), [](const char32_t& c){});
  } catch (const std::invalid_argument&) {
   return false;
  }
  return true;
 }

 // Third variant of is_valid_utf(): Specification of encoding via container type
 //
 // see also type_traits.h for is_container
 template<typename Container, std::enable_if_t<is_container_v<Container>, bool> = true>
 bool is_valid_utf(const Container& s)
 {
  typedef UTF<utf_iterator<typename Container::value_type, Container>, utf_back_insert_iterator<typename Container::value_type, Container>> UTF_Trait;
  
  try {
   std::for_each(UTF_Trait::begin(s), UTF_Trait::end(s), [](const char32_t& c){});
  } catch (const std::invalid_argument&) {
   return false;
  }
  return true;
 }

} // namespace unicode

