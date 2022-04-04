#pragma once

#include <string>

#include "u_Vector.h"
#include "u_Strings.h"

namespace decode {

    /// @return 0..15 if x = 0..F
    ///         999 otherwise
    unsigned int hexDigitValue(char32_t x);

    /// @return 0..7 if x = 0..7
    ///         999 otherwise
    inline unsigned int octDigitValue(char32_t x) {
        if (x >= '0' && x <= '7')
            return x - '0';
        return 999;
    }

    std::u32string_view normalizeEolSv(
            std::u32string_view x,
            std::u32string &cache);

    namespace dcpp {
        ///  @return [+] x is digit 0..9, Latin letter a..z A..Z, underscore _
        bool isAlnum(char32_t x);

        ///  @return [+] x is Latin letter a..z A..Z, or underscore _
        bool isAlpha(char32_t x);
    }

    /// Decodes C++
    /// "alpha\nbravo" → alpha<LF>bravo
    ///
    std::u32string cpp(std::u32string_view x);

}   // namespace decode
