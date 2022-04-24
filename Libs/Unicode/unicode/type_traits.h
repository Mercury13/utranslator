//
// Reichwein.IT Unicode Library
//
// Type traits
//

#pragma once

#include "utf_fwd.h"

#include <string>
#include <type_traits>

namespace unicode {

 using namespace detail;

 // helper traits
 
 template<typename T>
 struct is_encoding
 {
  static const bool value{std::is_empty_v<T>};
 };
 
 template<typename T>
 inline constexpr bool is_encoding_v {is_encoding<T>::value};

 template<typename T>
 struct is_container
 {
  static const bool value{!std::is_empty_v<T>};
 };
 
 template<typename T>
 inline constexpr bool is_container_v {is_container<T>::value};

 template<typename T>
 struct is_char
 {
  static const bool value{std::is_trivial_v<T> && std::is_scalar_v<T>};
 };
 
 template<typename T>
 inline constexpr bool is_char_v {is_char<T>::value};

 template<typename T>
 struct is_utf_encoding
 {
  static const bool value{std::is_same_v<T, UTF<utf_iterator<typename T::value_type>, utf_back_insert_iterator<typename T::value_type>>>};
 };

 template<typename T>
 inline constexpr bool is_utf_encoding_v {is_utf_encoding<T>::value};

 template<typename T>
 struct is_utf_8
 {
  static const bool value{std::is_same_v<T, UTF_8> || (std::is_trivial_v<T> && sizeof(T) == 1)};
 };
 
 template<typename T>
 inline constexpr bool is_utf_8_v {is_utf_8<T>::value};

 template<typename T>
 struct is_utf_16
 {
  static const bool value{std::is_same_v<T, UTF_16> || (std::is_trivial_v<T> && sizeof(T) == 2)};
 };
 
 template<typename T>
 inline constexpr bool is_utf_16_v {is_utf_16<T>::value};

 template<typename T>
 struct is_utf_32
 {
  static const bool value{std::is_same_v<T, UTF_32> || (std::is_trivial_v<T> && sizeof(T) == 4)};
 };
 
 template<typename T>
 inline constexpr bool is_utf_32_v {is_utf_32<T>::value};

} // namespace unicode
