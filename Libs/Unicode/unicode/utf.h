//
// Reichwein.IT Unicode Library
//
// Functions for support of UTF encodings
//
// Implementation of utf_iterator and utf_back_insert_iterator templates for
// validation and conversion via STL compatible iteration over standard
// containers
//

#pragma once

#include "utf_fwd.h"
#include "type_traits.h"

#include <list>
#include <string>
#include <stdexcept>

namespace unicode::detail {

 using namespace std::string_literals;

 template<size_t sequence_length, typename value_type>
 inline bool is_utf8_leading_byte(value_type byte) noexcept
 {
  static_assert(sequence_length <= 4);

  if constexpr(sequence_length == 1) {
   return !(byte & 0x80);
  } else {
   return (byte & static_cast<value_type>(0xFF << (7 - sequence_length))) == static_cast<value_type>(0xFF << (8 - sequence_length));
  }
 }

 template<typename value_type>
 inline bool is_utf8_followup_byte(value_type b) noexcept
 {
  return (b & 0b11000000) == 0b10000000;
 }

 template<typename value_type, typename... Tbytes>
 inline bool is_utf8_sequence(value_type byte0, Tbytes... bytes) noexcept
 {
  constexpr auto sequence_length{sizeof...(Tbytes) + 1};

  static_assert(sequence_length <= 4, "UTF-8 sequences of 1 through 4 code units are supported");

  return is_utf8_leading_byte<sequence_length>(byte0) &&
         (... && is_utf8_followup_byte(bytes)); // left fold for linear evaluation from left to right
 }

 template<typename T, typename std::enable_if_t<is_utf_8_v<T>, bool> = true>
 inline bool validate_utf(const std::basic_string<T>& s)
 {
  size_t i{};
  size_t size{s.size()};
  while (i < size) {
   if (is_utf8_sequence(s[i])) {
    i++;
   } else if ((i < size - 1) && is_utf8_sequence(s[i], s[i + 1])) {
    i += 2;
   } else if ((i < size - 2) && is_utf8_sequence(s[i], s[i + 1], s[i + 2])) {
    if (((s[i] & 0xF) == 0xD) && ((s[i + 1] & 0x20) == 0x20))
     return false; // Reserved for UTF-16 surrogates: 0xD800..0xDFFF
    i += 3;
   } else if ((i < size - 3) && is_utf8_sequence(s[i], s[i + 1], s[i + 2], s[i + 3])) {
    if ((((s[i] & 7) << 2) | ((s[i + 1] >> 4) & 3)) >= 0x11)
     return false; // Unicode too big above 0x10FFFF
    i += 4;
   } else
#if __cplusplus >= 202002L
   [[unlikely]]
#endif
    return false;
  }
  return true;
 }

 template<typename value_type, typename... Twords>
 inline bool is_utf16_sequence(value_type word0, Twords... words) noexcept
 {
  constexpr auto sequence_length{sizeof...(Twords) + 1};

  static_assert(sequence_length <= 2, "UTF-16 sequences of only 1 or 2 code units are supported");

  if constexpr(sequence_length == 1) {
   return is_valid_unicode(word0);
  } else {
   char16_t unit0 {static_cast<char16_t>(word0)};
   char16_t unit1 {static_cast<char16_t>((words, ...))};
   return (unit0 & 0xFC00) == 0xD800 && (unit1 & 0xFC00) == 0xDC00;
  }
 }

 template<typename T, typename std::enable_if_t<is_utf_16_v<T>, bool> = true>
 inline bool validate_utf(const std::basic_string<T>& s)
 {
  size_t i{};
  size_t size{s.size()};
  while (i < size) {
   if (is_utf16_sequence(s[i])) {
    i++;
   } else if ((i < size - 1) && is_utf16_sequence(s[i], s[i + 1])) {
    i += 2;
   } else
#if __cplusplus >= 202002L
    [[unlikely]]
#endif
    return false;
  }
  return true;
 }

 template<typename T, typename std::enable_if_t<is_utf_32_v<T>, bool> = true>
 inline bool validate_utf(const std::basic_string<T>& s)
 {
  for (auto i: s)
   if (!is_valid_unicode(i))
    return false;
  return true;
 }

 template<size_t sequence_length, typename value_type>
 inline char32_t decode_utf8_leading_byte(value_type b) noexcept
 {
  return static_cast<char32_t>(b & (0b1111111 >> sequence_length)) << ((sequence_length - 1) * 6);
 }

 template<typename value_type>
 inline char32_t decode_utf8_followup_byte(value_type b) noexcept
 {
  return static_cast<char32_t>(b & 0b00111111);
 }

 template<typename value_type, typename... Targs>
 inline char32_t decode_utf8_followup_byte(value_type b, Targs... bytes) noexcept
 {
  return decode_utf8_followup_byte(b) << (6 * sizeof...(Targs)) | decode_utf8_followup_byte(bytes...);
 }

 template<typename value_type, typename... Targs>
 inline char32_t decode_utf8_sequence(value_type b, Targs... bytes) noexcept
 {
  size_t constexpr sequence_length{sizeof...(Targs) + 1};

  static_assert(sequence_length <= 4);

  if constexpr (sequence_length == 1)
   return b;
  else
   return decode_utf8_leading_byte<sequence_length>(b) | decode_utf8_followup_byte(bytes...);
 }

 template<typename T, typename Container>
 struct utf_iterator
 {
  static_assert(is_utf_8_v<T> || is_utf_16_v<T> || is_utf_32_v<T>);

  typedef T value_type;
  typedef char32_t internal_type;
  typedef char32_t& reference;
  typedef char32_t* pointer;
  typedef size_t difference_type;
  typedef std::input_iterator_tag iterator_category;
  typedef Container string_type;

  utf_iterator(const typename string_type::const_iterator& cbegin, const typename string_type::const_iterator& cend):
   iterator(cbegin), end_iterator(cend)
  {
  }

  utf_iterator(const utf_iterator& other) = default;
  utf_iterator& operator=(const utf_iterator& other) = default;

  inline size_t remaining_code_units() const noexcept
  {
   return std::distance(iterator, end_iterator);
  }

  template<size_t index>
  inline value_type get_code_unit() const noexcept
  {
   if constexpr (std::is_same_v<Container, typename std::list<value_type>>) {
    // std::list doesn't support it + n
    auto it{iterator};
    std::advance(it, index);
    return *it;
   } else {
    return *(iterator + index);
   }
  }

  template<typename... Tbytes>
  inline internal_type calculate_utf8_value(Tbytes... bytes)
  {
   size_t constexpr sequence_length{sizeof...(Tbytes)};
   static_assert(sequence_length >= 1 && sequence_length <= 4);

   if (is_utf8_sequence(bytes...)) {
    std::advance(iterator, sequence_length);
    internal_type result{decode_utf8_sequence(bytes...)};
    if (!unicode::is_valid_unicode<sequence_length * 6>(result))
#if __cplusplus >= 202002L
     [[unlikely]]
#endif
     throw std::invalid_argument("Invalid Unicode character: "s + std::to_string(static_cast<uint32_t>(result)));
    return result;
   } else {
    if constexpr(sequence_length <= 3) { // template recursion break condition: UTF-8 has 1..4 code units
     if (remaining_code_units() < sequence_length + 1)
#if __cplusplus >= 202002L
      [[unlikely]]
#endif
      throw std::invalid_argument("Bad input: Not enough bytes left for decoding UTF-8 sequence");

     return calculate_utf8_value(bytes..., static_cast<utf8_t>(get_code_unit<sequence_length>()));
    } else
     throw std::invalid_argument("Bad UTF-8 input: Invalid 4 byte sequence");
   }
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_8_v<X>, bool> = true>
  inline internal_type calculate_value()
  {
   return calculate_utf8_value(static_cast<utf8_t>(get_code_unit<0>()));
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_16_v<X>, bool> = true>
  inline internal_type calculate_value()
  {
   char16_t unit0 {static_cast<char16_t>(get_code_unit<0>())};

   if (is_valid_unicode(unit0)) { // 1 unit (BMP Basic Multilingual Plane)
    std::advance(iterator, 1);
    return unit0;
   } else {
    if (remaining_code_units() < 2)
#if __cplusplus >= 202002L
     [[unlikely]]
#endif
     throw std::invalid_argument("Bad input: Continuation of first UTF-16 unit missing");

    char16_t unit1 {static_cast<char16_t>(get_code_unit<1>())};
    if ((unit0 & 0xFC00) != 0xD800 || (unit1 & 0xFC00) != 0xDC00)
#if __cplusplus >= 202002L
     [[unlikely]]
#endif
     throw std::invalid_argument("Bad input: 2 malformed UTF-16 surrogates");

    std::advance(iterator, 2);
    return (static_cast<internal_type>(unit0 & 0x03FF) << 10 | (unit1 & 0x03FF)) + 0x10000;
   }
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_32_v<X>, bool> = true>
  inline internal_type calculate_value()
  {
   internal_type result {static_cast<internal_type>(get_code_unit<0>())};

   if (!unicode::is_valid_unicode(result))
#if __cplusplus >= 202002L
    [[unlikely]]
#endif
    throw std::invalid_argument("Invalid Unicode character: "s + std::to_string(static_cast<uint32_t>(result)));

   std::advance(iterator, 1);

   return result;
  }

  // pre-increment
  utf_iterator& operator++()
  {
   return *this;
  }

  bool operator!=(const utf_iterator& other) const
  {
   return std::distance(iterator, end_iterator) != std::distance(other.iterator, other.end_iterator);
  }

  internal_type operator*()
  {
   return calculate_value();
  }

  utf_iterator& operator+=(size_t distance)
  {
   std::advance(iterator, distance);
   return *this;
  }

  size_t operator-(const utf_iterator& other) const
  {
   return iterator - other.iterator;
  }

 private:
  typename string_type::const_iterator iterator;
  typename string_type::const_iterator end_iterator;
 };

 // n is number of UTF-8 bytes in sequence
 template<size_t n, typename From, typename To>
 inline To utf8_byte0_of(const From& value)
 {
  return (value >> 6 * (n - 1)) | (0xFF << (8 - n));
 }

 // n is index of 6-bit groups, counting from bit 0
 template<size_t n, typename From, typename To>
 inline To utf8_trailing_byte(const From& value)
 {
  return ((value >> n * 6) & 0b111111) | 0b10000000;
 }

 // calculate UTF-8 sequence byte for m >= 2 bytes sequences (i.e. non-ASCII)
 // assume value to be valid Unicode value for given byte position
 template<size_t n, size_t m, typename From, typename To>
 inline To utf8_byte_n_of_m(const From& value)
 {
  if constexpr (n == 0)
   return utf8_byte0_of<m, From, To>(value);
  else
   return utf8_trailing_byte<m - n - 1, From, To>(value);
 }

 template<typename T, typename Container>
 struct utf_back_insert_iterator
 {
  static_assert(is_utf_8_v<T> || is_utf_16_v<T> || is_utf_32_v<T>);

  typedef T value_type;
  typedef char32_t internal_type;
  typedef Container string_type;
  typedef utf_back_insert_iterator& reference;
  typedef utf_back_insert_iterator* pointer;
  typedef size_t difference_type;
  typedef std::output_iterator_tag iterator_category;

  utf_back_insert_iterator(string_type& s): s(s) {}

  utf_back_insert_iterator& operator=(const utf_back_insert_iterator& other)
  {
   if (std::addressof(other.s) != std::addressof(s))
    throw std::runtime_error("utf_back_insert_iterator assignment operator actually called! Iterator should not be assigned to.");

   return *this;
  }

  // no-op
  reference operator++()
  {
   return *this;
  }

  // support *x = value, together with operator=()
  reference operator*()
  {
   return *this;
  }

  template<typename... Args>
  inline void append(Args&&... args)
  {
   if constexpr (std::is_same_v<Container, typename std::basic_string<value_type>>) {
    s.append({args...});
   } else {
    (s.emplace_back(args), ...);
   }
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_8_v<X>, bool> = true>
  inline void append_utf(const internal_type& value)
  {
   using Y = internal_type;
   if (value < 0x80) { // 1 byte
    append(static_cast<value_type>(value));
   } else if (value < 0x800) { // 2 bytes
    append(utf8_byte_n_of_m<0,2,Y,X>(value), utf8_byte_n_of_m<1,2,Y,X>(value));
   } else if (value < 0x10000) { // 3 bytes
    append(utf8_byte_n_of_m<0,3,Y,X>(value), utf8_byte_n_of_m<1,3,Y,X>(value), utf8_byte_n_of_m<2,3,Y,X>(value));
   } else { // 4 bytes
    // expect value to be already valid Unicode values (checked in input iterator)
    append(utf8_byte_n_of_m<0,4,Y,X>(value), utf8_byte_n_of_m<1,4,Y,X>(value), utf8_byte_n_of_m<2,4,Y,X>(value), utf8_byte_n_of_m<3,4,Y,X>(value));
   }
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_16_v<X>, bool> = true>
  inline void append_utf(const internal_type& value)
  {
   if (value <= 0xFFFF) { // expect value to be already valid Unicode values (checked in input iterator)
    append(static_cast<value_type>(value));
   } else {
    internal_type value_reduced{value - 0x10000};
    append(static_cast<value_type>((value_reduced >> 10) + 0xD800), static_cast<value_type>((value_reduced & 0x3FF) + 0xDC00));
   }
  }

  template<class X = value_type, typename std::enable_if_t<is_utf_32_v<X>, bool> = true>
  inline void append_utf(const internal_type& value)
  {
   // expect value to be already valid Unicode values (checked in input iterator)
   append(static_cast<value_type>(value));
  }

  reference operator=(const internal_type& value)
  {
   append_utf(value);
   return *this;
  }

 private:
  typename utf_back_insert_iterator::string_type& s;
 };

} // namespace unicode::detail

namespace unicode {
 
 // Encoding for convert() and UTF-*
 template<typename InputIt, typename OutputIt>
 struct UTF
 {
  typedef typename OutputIt::value_type value_type;
  typedef typename InputIt::string_type string_type;

  static InputIt begin(const typename InputIt::string_type& s)
  {
   return InputIt{s.cbegin(), s.cend()};
  }

  static InputIt end(const typename InputIt::string_type& s)
  {
   return InputIt{s.cend(), s.cend()};
  }

  static OutputIt back_inserter(typename OutputIt::string_type& s)
  {
   return OutputIt(s);
  }
 };

 // Helper to get correct Encoding from char type, e.g. Encoding<typename decltype(s)::value_type>::type or Encoding_t<typename decltype(s)::value_type>
 template<typename T>
 struct Encoding
 {
 };

 template<>
 struct Encoding<utf8_t>
 {
  typedef UTF_8 type;
 };

 template<>
 struct Encoding<char16_t>
 {
  typedef UTF_16 type;
 };

 template<>
 struct Encoding<char32_t>
 {
  typedef UTF_32 type;
 };

 template<typename T>
 using Encoding_t = typename Encoding<T>::type;

} // namespace unicode

