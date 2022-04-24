//
// Reichwein.IT Unicode Library
//
// Forward declarations for utf.h - Functions for reading and writing UTF
// encodings
//

#pragma once

#include "types.h"

#include <string>

namespace unicode::detail {

 template<typename T, typename Container=std::basic_string<T>>
 struct utf_iterator;

 template<typename T, typename Container=std::basic_string<T>>
 struct utf_back_insert_iterator;

} // namespace unicode::detail

namespace unicode {

 template<typename InputIt, typename OutputIt>
 struct UTF;

 // Encoding for convert()
 typedef UTF<utf_iterator<utf8_t>, utf_back_insert_iterator<utf8_t>> UTF_8;
 typedef UTF<utf_iterator<char16_t>, utf_back_insert_iterator<char16_t>> UTF_16;
 typedef UTF<utf_iterator<char32_t>, utf_back_insert_iterator<char32_t>> UTF_32;

} // namespace unicode

