#pragma once

#include <string>

#include "u_Vector.h"
#include "u_Strings.h"

namespace decode {

    /// @return 0..15 if x = 0..F
    ///         -1 otherwise
    int hexDigitValue(char x);

    /// @return 0..9 if x = 0..9
    ///         -1 otherwise
    inline int decDigitValue(char x) {
        if (x >= '0' && x <= '9')
            return x - '0';
        return -1;
    }

    template <class Ch, class Tr>
    std::string_view normalizeEolSv(
            std::basic_string_view<Ch, Tr> x,
            std::basic_string<Ch, Tr> cache)
    {
        using Sv = std::basic_string_view<Ch, Tr>;
        if (x.find('\r') == Sv::npos)
            return x;
        cache = x;
        str::replace(cache, "\r\n", "\n");
        str::replace(cache, '\r', '\n');
        return cache;
    }

    namespace dcpp {
        ///  @return [+] x is digit 0..9, Latin letter a..z A..Z, underscore _
        bool isAlnum(char x);

        ///  @return [+] x is Latin letter a..z A..Z, or underscore _
        bool isAlpha(char x);
    }

    /// Decodes C++
    /// "alpha\nbravo" → alpha<LF>bravo
    ///
    std::string cpp(std::string_view x);

}   // namespace decode
