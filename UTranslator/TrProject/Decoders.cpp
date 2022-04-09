// My header
#include "Decoders.h"

// C++
#include <regex>

constexpr char32_t PARA_SEP_32 = 0x2029;      // U+2029 paragraph separator
constexpr wchar_t PARA_SEP_16 = PARA_SEP_32;

std::u32string_view decode::normalizeEolSv(
        std::u32string_view x,
        std::u32string &cache)
{
    if (x.find(U'\r') == std::u32string_view::npos
            && x.find(PARA_SEP_32) == std::u32string_view::npos)
        return x;
    cache = x;
    str::replace(cache, U"\r\n", U"\n");
    str::replace(cache, U'\r', U'\n');
    str::replace(cache, PARA_SEP_32, U'\n');
    return cache;
}


std::wstring_view decode::normalizeEolSv(
        std::wstring_view x,
        std::wstring &cache)
{
    if (x.find(u'\r') == std::u32string_view::npos
            && x.find(PARA_SEP_16) == std::u32string_view::npos)
        return x;
    cache = x;
    str::replace(cache, L"\r\n", L"\n");
    str::replace(cache, L'\r', L'\n');
    str::replace(cache, PARA_SEP_16, L'\n');
    return cache;
}


bool decode::dcpp::isAlnum(char32_t x)
{
    return (x >= U'0' && x <= U'9')
        || (x >= U'a' && x <= U'z')
        || (x >= U'A' && x <= U'Z')
        || ( x == U'_');
}


bool decode::dcpp::isAlpha(char32_t x)
{
    return (x >= U'a' && x <= U'z')
        || (x >= U'A' && x <= U'Z')
        || ( x == U'_');
}


unsigned int decode::hexDigitValue(char32_t x)
{
    if (x >= U'0' && x <= U'9')
        return x - U'0';
    if (x >= U'A' && x <= U'F')
        return x - ('A' - 10);
    if (x >= U'a' && x <= U'f')
        return x - (U'a' - 10);
    return 999;
}


namespace {
    inline char32_t convertCp(char32_t c) {
        if (c < 0xD800) {
            if (c == U'\r')
                return U'\n';
            return c;
        } else {
            return (c >= 0xE000 && c <= 0x10FFFF) ? c : 0;
        }
    }

    void appendCode(std::u32string& r, char32_t c)
    {
        if (auto q = convertCp(c))
            r += q;
    }

    enum class CppState {
        SPACE,      // ␣␣      prefixStart YES
        SPACE_TRAIL,// "x"␣␣   prefixStart YES
        PREFIX,     // ␣␣ab    prefixStart YES
        INSIDE,     // "x
        SLASH,      // "x/         BACK slash here
        OCT,        // "x/2        BACK slash here
        HEX,        // "x/uA       BACK slash here
        SUFFIX,     // "x"ab
            // Four states for unquoted string
            // When we see backslash outside quotes, we fall here!
            // Same order as INSIDE/SLASH/OCT/HEX
        OUTSIDE,    // ␣␣1+
        //OUT_SLASH,
        //OUT_OCT,
        //OUT_HEX,
    };

    enum class CppBase {
        INSIDE = static_cast<int>(CppState::INSIDE),
        OUTSIDE = static_cast<int>(CppState::INSIDE),
    };

    enum class CppDelta { INSIDE, SLASH, OCT, HEX };

    [[maybe_unused]] constexpr CppState operator + (CppBase base, CppDelta delta)
        { return CppState(static_cast<int>(base) + static_cast<int>(delta)); }
}   // anon namespace


std::u32string decode::cpp(std::u32string_view x)
{
    std::u32string cache;
    x = normalizeEolSv(x, cache);

    const char32_t* p = x.data();
    const char32_t* end = p + x.length();

    std::u32string r;

    CppState state = CppState::SPACE;
    const char32_t* prefixStart = p;
    char32_t charCode = 0;
    int nCodeCharsRemaining = 0;

    auto dropCode = [&](CppState revertState) {     // Both OCT and HEX → prefixStart NO
        appendCode(r, charCode);
        state = revertState;      // prefixStart keep NO
        nCodeCharsRemaining = 0;
        charCode = 0;
    };

    auto processCode = [&](char32_t c, unsigned base, CppState revertState) {
        if (auto val = hexDigitValue(c); val < base) {
            charCode = charCode * base + val;
            if ((--nCodeCharsRemaining) == 0) {
                dropCode(revertState);
            }
        } else {
            dropCode(revertState);
            r += c;
        }
    };

    auto processPostSlash = [&](char32_t c, CppBase base) {
        state = base + CppDelta::INSIDE;              // prefixStart keep NO
                // Maybe we’ll change state to another one!!
        switch (c) {
        case U'a': r += '\a'; break;
        case U'b': r += '\b'; break;
        case U't': r += '\t'; break;
        case U'n':
        case U'r': r += '\n'; break;     // both /n and /r yield LF!!
        case U'f': r += '\f'; break;
        case U'v': r += '\v'; break;
        case U'U':
            nCodeCharsRemaining = 8;
            charCode = 0;
            state = base + CppDelta::HEX;         // prefixStart keep NO
            break;
        case U'u':
            nCodeCharsRemaining = 4;
            charCode = 0;
            state = base + CppDelta::HEX;         // prefixStart keep NO
            break;
        case U'x':
            nCodeCharsRemaining = 2;
            charCode = 0;
            state = base + CppDelta::HEX;         // prefixStart keep NO
            break;
        case U'\n': break;               // do nothing, though is is strange string
        default:
            if (auto val = octDigitValue(c); val < 8) {
                charCode = val;
                nCodeCharsRemaining = 2;    // up to 3, incl. this one
                state = base + CppDelta::OCT;     // prefixStart keep NO
            } else {
                r += c;
            }
        }
    };

    for (; p != end; ++p) {
        auto c = *p;
        switch (state) {
        case CppState::SPACE:
        case CppState::SPACE_TRAIL: // prefixStart YES
            switch (c) {
            case U' ':
            case U'\n':
                break;  // do nothing
            case U'"':
                state = CppState::INSIDE;      // prefixStart YES→NO, do not dump
                prefixStart = nullptr;
                break;
            case U',':
            case U';':
            case U')':
            case U'}':
                if (state == CppState::SPACE_TRAIL)
                    break;                  // do nothing in SPACE_TRAIL mode
                [[fallthrough]];
            default:
                if (dcpp::isAlpha(c)) {
                    state = CppState::PREFIX;  // prefixStart YES→YES, keep on stockpiling
                } else {
                    if (state == CppState::SPACE)
                        r.append(prefixStart, p);
                    r += c;
                    prefixStart = nullptr;
                    state = CppState::OUTSIDE; // prefixStart YES→NO, dump prefix+char (prefix not in SPACE_TRAIL)
                }
            }
            break;
        case CppState::PREFIX: // prefixStart YES
            switch (c) {
            case U' ':                       // ␣␣ab␣
            case U'\n':
                state = CppState::SPACE;       // prefixStart YES→YES, dump
                r.append(prefixStart, p);
                prefixStart = p;
                break;
            case U'"':                       // ␣␣ab"
                state = CppState::INSIDE;      // prefixStart YES→NO, do not dump
                prefixStart = nullptr;
                break;
            default:
                if (dcpp::isAlnum(c)) {     // ␣␣ab8
                    // do nothing, keep on stockpiling
                } else {                    // ␣␣ab+
                    state = CppState::OUTSIDE; // prefixStart YES→NO, dump prefix+char
                    r.append(prefixStart, p);
                    r += c;
                    prefixStart = nullptr;
                }
            }
            break;
        case CppState::OUTSIDE: // prefixStart NO
            switch (c) {
            case U' ':                       // ␣␣1+␣
            case U'\n':
                state = CppState::SPACE;       // prefixStart NO→YES
                prefixStart = p;
                break;
            case U'"':                       // ␣␣1+"
                state = CppState::INSIDE;      // prefixStart keep NO
                break;
            default:
                if (dcpp::isAlpha(c)) {     // ␣␣1+a
                    state = CppState::PREFIX;  // prefixStart NO→YES
                    prefixStart = p;
                } else {                    // ␣␣1+2
                    r += c;
                }
            }
            break;
        case CppState::INSIDE: // prefixStart NO
            switch (c) {
            case U'"':
                state = CppState::SUFFIX;  // prefixStart keep NO
                break;
            case U'\\':
                state = CppState::SLASH;   // prefixStart keep NO
                break;
            default:
                r += c;
            }
            break;
        case CppState::SLASH: // prefixStart NO
            processPostSlash(c, CppBase::INSIDE);
            break;
        case CppState::HEX:    // prefixStart NO
            processCode(c, 16, CppState::INSIDE);
            break;
        case CppState::OCT:    // prefixStart NO
            processCode(c, 8, CppState::INSIDE);
            break;
        case CppState::SUFFIX:
            switch (c) {
            case U' ':                          // "x"ab␣
            case U'\n':
            case U',':                          // "x"ab,  do the same
            case U')':                          // "x"ab)  do the same
            case U'}':                          // "x"ab}  do the same
            case U';':                          // "x"ab;  do the same
                state = CppState::SPACE_TRAIL;     // prefixStart NO→YES
                prefixStart = p;
                break;
            default:
                if (dcpp::isAlnum(c)) {         // "x"ab8
                    //  do nothing
                } else {                        // "x"ab+
                    state = CppState::OUTSIDE;     // prefixStart keep NO
                    r += c;
                }
            }
            break;
        }   // Big switch (state)
    }
    if (prefixStart && state != CppState::SPACE_TRAIL) {
        r.append(prefixStart, end);
    }
    if (nCodeCharsRemaining > 0)
        appendCode(r, charCode);
    return r;
}


std::wstring decode::htmlBr(std::wstring_view x)
{
    std::wstring cache;
    x = normalizeEolSv(x, cache);

    if (x.empty())
        return std::wstring{x};

    std::wregex rex(LR"(<br[ ]*[/]?>(?!\n))");
    std::wstring r;
    std::regex_replace(
            std::back_inserter(r),
            x.begin(), x.end(),
            rex,
            L"<br>\n");
    return r;
}
