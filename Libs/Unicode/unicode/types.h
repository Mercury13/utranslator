//
// Reichwein.IT Unicode Library
//
// Basic types
//

#pragma once

// Definition of utf8_t as abstraction from char and char8_t, when available
//
// Be aware of char being signed on common architectures, while char8_t is
// unsigned.
#ifdef __cpp_char8_t
 // char8_t available in C++20
 typedef char8_t utf8_t;
#else
 // fallback to char
 typedef char utf8_t;
#endif
typedef char iso_t;

