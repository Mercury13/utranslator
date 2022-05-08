#pragma once

#include <string_view>

#define S8(x)  u8"" x

#define STR_UNTITLED       "(Untitled)"
#define STR_UNTRANSLATED   "(Not yet translated)"
#define STR_EMPTY_STRING   "(Empty string)"

constexpr const std::string_view langList[] = {
    "be", "cz", "cn", "de", "en", "es", "fr", "he", // Hebrew
    "hi",   // Hindi
    "jp", "ru", "uk"
};
