//
// Reichwein.IT Unicode Library
//
// ISO 8895 (-1 and -15) handling functions (i.e. Latin-1 and Latin-9)
//
// Implementation of iso_iterator for reading individual Unicode code points
// from an string or container input, and a iso_back_insert_iterator for
// writing them to the destination.
//
// The design is made to be compatible to the respective iterators in utf.h
// to make it easy to combine them.
//

#pragma once

#include "types.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace unicode::detail {

 using namespace std::string_literals;

 typedef std::unordered_map<iso_t, char32_t> iso_map_type;
 typedef std::unordered_map<char32_t, iso_t> iso_map_type_reverse;

 // ISO-8859-1 is lower 8-bit of Unicode, so no exceptions necessary
 static inline iso_map_type iso_8859_1_map;

 // ISO-8859-15 is lower 8-bit of Unicode, except for:
 static inline iso_map_type iso_8859_15_map {
  { '\xA4', U'\u20AC' }, // €
  { '\xA6', U'\u0160' }, // Š
  { '\xA8', U'\u0161' }, // š
  { '\xB4', U'\u017D' }, // Ž
  { '\xB8', U'\u017E' }, // ž
  { '\xBC', U'\u0152' }, // Œ
  { '\xBD', U'\u0153' }, // œ
  { '\xBE', U'\u0178' }, // Ÿ
 };

 inline iso_map_type_reverse reverse_iso_map(const iso_map_type& map) {
  iso_map_type_reverse result;
  std::for_each(map.cbegin(), map.cend(),
                [&](const iso_map_type::value_type& pair)
                 {
                  result.emplace(pair.second, pair.first);
                  result.emplace(static_cast<char32_t>(static_cast<uint8_t>(pair.first)), 0); // map invalid characters to a known non-mapped value as marker
                 });
  return result;
 }

 static inline iso_map_type_reverse iso_8859_15_map_reverse { reverse_iso_map(iso_8859_15_map) };
 static inline iso_map_type_reverse iso_8859_1_map_reverse { reverse_iso_map(iso_8859_1_map) };

 template<unicode::detail::iso_map_type& Map=iso_8859_1_map, typename Container=std::basic_string<iso_t>>
 struct iso_iterator {
  typedef iso_t value_type;
  typedef char32_t internal_type;
  typedef char32_t& reference;
  typedef char32_t* pointer;
  typedef size_t difference_type;
  typedef std::input_iterator_tag iterator_category;
  typedef typename Container::const_iterator iterator;
  typedef Container string_type;

  iso_iterator(const iterator& it): m_it(it) {}

  // pre-increment
  iso_iterator& operator++()
  {
   ++m_it;
   return *this;
  }

  bool operator!=(const iso_iterator& other) const
  {
   return m_it != other.m_it;
  }

  // return reference?
  internal_type operator*() const
  {
   value_type value{*m_it};

   if constexpr(std::addressof(Map) != std::addressof(iso_8859_1_map)) // mapping of 128 <= x <= 255 if needed
   {
    auto it{Map.find(value)};
    if (it != Map.end())
     return it->second;
   }
   return static_cast<internal_type>(static_cast<uint8_t>(value));
  }

  iso_iterator& operator+=(size_t distance)
  {
   std::advance(m_it, distance);
   return *this;
  }

  difference_type operator-(const iso_iterator& other) const
  {
   return m_it - other.m_it;
  }

 private:
  iterator m_it;
 };

 template<unicode::detail::iso_map_type_reverse& Map=iso_8859_1_map_reverse, typename Container=std::basic_string<iso_t>>
 struct iso_back_insert_iterator {
  typedef iso_back_insert_iterator& reference;
  typedef iso_back_insert_iterator* pointer;
  typedef size_t difference_type;
  typedef iso_t value_type;
  typedef char32_t internal_type;
  typedef std::output_iterator_tag iterator_category;
  typedef Container string_type;
  
  iso_back_insert_iterator(string_type& s): s(s) {}

  iso_back_insert_iterator& operator=(const iso_back_insert_iterator& other)
  {
   if (std::addressof(other.s) != std::addressof(s))
    throw std::runtime_error("iso_back_insert_iterator assignment operator actually called! Iterator should not be assigned to.");

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

  reference operator=(const internal_type& value)
  {
   if constexpr(std::addressof(Map) != std::addressof(iso_8859_1_map_reverse)) // mapping back to 128 <= x <= 255 if needed
   {
    auto it{Map.find(value)};
    if (it != Map.end()) {
     if (it->second == 0) // marker for non-mappable character found
      throw std::invalid_argument("Bad Unicode value to map to ISO 8859-15: "s + std::to_string(static_cast<uint32_t>(value)));
     s.push_back(it->second);
     return *this;
    }
   }

   if (value > 255)
    throw std::invalid_argument("Bad ISO 8859 value above 255: "s + std::to_string(static_cast<uint32_t>(value)));

   s.push_back(static_cast<typename iso_back_insert_iterator::value_type>(value));
   return *this;
  }

 private:
  typename iso_back_insert_iterator::string_type& s;
 };

} // namespace unicode::detail

namespace unicode {

 using namespace detail;

 // Encoding for convert() and ISO-8859-*
 template<typename InputIt, typename OutputIt>
 struct ISO_8859
 {
  typedef iso_t value_type;
  typedef typename InputIt::string_type string_type;

  static InputIt begin(const typename InputIt::string_type& s)
  {
   return InputIt(s.cbegin());
  }

  static InputIt end(const typename InputIt::string_type& s)
  {
   return InputIt(s.cend());
  }

  static OutputIt back_inserter(typename OutputIt::string_type& s)
  {
   return OutputIt(s);
  }
 };

 // Encoding for convert()
 typedef ISO_8859<iso_iterator<>, iso_back_insert_iterator<>> ISO_8859_1;
 typedef ISO_8859<iso_iterator<iso_8859_15_map>, iso_back_insert_iterator<iso_8859_15_map_reverse>> ISO_8859_15;

} // namespace unicode

