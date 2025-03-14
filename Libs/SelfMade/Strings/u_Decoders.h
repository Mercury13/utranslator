#pragma once

#include <string>
#include <iostream>

#include "u_EnumSize.h"

template<class Ec>
constexpr Ec ecIf(bool x) {
    if constexpr(static_cast<int>(Ec::NO) == 0
              && static_cast<int>(Ec::YES) == 1) {
        return static_cast<Ec>(x);
    } else {
        return x ? Ec::YES : Ec::NO;
    }
}

enum class Enquote : unsigned char { NO, YES };

namespace escape {

    DEFINE_ENUM_TYPE_IN_NS(escape, LineBreakMode, unsigned char,
        BANNED,         ///< Line-breaks banned
        C_CR,           ///< C mode: break = /r, / = //   (actually BACKslash here)
        C_LF,           ///< C mode: break = /n, / = //   (actually BACKslash here)
        SPECIFIED_TEXT  ///< Specified character that’s banned in text
    )

    DEFINE_ENUM_TYPE_IN_NS(escape, SpaceMode, unsigned char,
        BARE,
        DELIMITED,
        QUOTED,
        SLASH_SPACE
    )

    ///  Principles of escaping line-breaks
    struct Text {
        static constexpr std::u8string_view DEFAULT_LINE_BREAK_TEXT = u8"^";
        static constexpr std::u8string_view DEFAULT_SPACE_DELIMITER = u8"|";
        LineBreakMode lineBreak = LineBreakMode::BANNED;
        std::u8string lineBreakText { DEFAULT_LINE_BREAK_TEXT };
        SpaceMode space;
        std::u8string spaceDelimiter { DEFAULT_SPACE_DELIMITER };

        std::u8string bannedSubstring() const;
        static void writeQuoted(std::ostream& os, std::u8string_view x);
        void writeSimpleString(std::ostream& os, std::u8string_view x) const;
        void write(std::ostream& os, std::u8string_view x, std::u8string& cache) const;

        void setLineBreakText(std::u8string_view x);
        std::u8string_view visibleLineBreakText() const noexcept;

        void setSpaceDelimiter(std::u8string_view x);

        /// @return  space delimiter visible in UI
        ///          (default if not demimited)
        std::u8string_view visibleSpaceDelimiter() const noexcept;

        /// @return  space delimiter that actually works
        ///          (empty if not demimited)
        std::u8string_view activeSpaceDelimiter() const noexcept;

        std::u8string unescape(std::u8string_view text) const;
        std::u8string unescapeMaybeQuoted(std::u8string_view text) const;
    };

    enum class Spaces { NO, YES };

    std::u8string_view cppSv(
            std::u8string_view x,
            std::u8string& cache,
            char8_t lf,
            escape::Spaces escapeSpaces = escape::Spaces::NO,
            Enquote enquote = Enquote::NO);

}

namespace decode {

    enum class BomType { NONE, UTF8, UTF16BE, UTF16LE };

    ///
    /// \brief detectBom
    ///    Reads and skips byte order mark
    /// \return BOM type
    ///
    BomType detectBom(std::istream& is);

    class IniCallback {     // interface
    public:
        virtual void onGroup(std::u8string_view) = 0;
        /// @warning  decode rawValue as you want!!
        ///           No text formats here!
        virtual void onVar(std::u8string_view name, std::u8string_view rawValue) = 0;
        /// @warning  Also on bad line
        virtual void onEmptyLine() {}
        virtual void onComment(std::u8string_view) {}
        virtual ~IniCallback() = default;
    };

    void ini(std::istream& is, IniCallback& cb);


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
    std::wstring_view normalizeEolSv(
            std::wstring_view x,
            std::wstring &cache);

    namespace dcpp {
        ///  @return [+] x is digit 0..9, Latin letter a..z A..Z, underscore _
        bool isAlnum(char32_t x);

        ///  @return [+] x is Latin letter a..z A..Z, or underscore _
        bool isAlpha(char32_t x);
    }

    enum class MaybeQuoted { NO, YES };

    /// Lighter C++ decode
    /// Supports:
    /// • \s for space
    ///
    /// Does NOT support:
    /// • C++ syntax peculiarities: prefixes, suffixes, raw strings, "str" "str"
    /// • \xXX, \oXXX, \uXXXX
    /// • trails of source code "xxxx" },
    ///
    /// @warning Special bhv when maybeQuoted+ → cannot make 2 functions
    ///               "\" → ",  \ = everything
    /// @warning Is NOT bug-compatible with decodeCpp
    ///
    /// @param [in] maybeQuoted  [+] the string is possibly quoted    ///
    ///
    std::u8string cppLite(std::u8string_view x, MaybeQuoted maybeQuoted);

    ///
    /// Decoding of "some ""quoted"" string"
    ///
    std::u8string quoted(std::u8string_view x);

    /// Decodes C++
    /// "alpha\nbravo" → alpha<LF>bravo
    /// Why U32: handle \U00012345
    ///
    /// Additional things:
    /// • starting and ending IDs: u8"text"sv
    /// • trailing chars: )},;
    ///
    /// NOT checked right now
    /// • Raw strings   R"(abc\ndef)"
    /// • Escape seqs w/o quotes   abc\ndef
    /// • Space policy when input has both quoted and unquoted text  abc "def" ghi
    ///
    std::u32string cpp(std::u32string_view x);

    /// Converts <br> → <br><LF>
    /// Why system-dependent wide…
    ///   • Unicode, better not UTF-8: QPlainTextEdit’s selection uses raw data, U+2029 inside
    ///   • Not C++20 u8string: Qt supports it in 6.2+ that dropped W7
    ///   • Locales: standard C++ regex
    ///
    std::wstring htmlBr(std::wstring_view x);

    /// Converts <p> → <LF><LF>
    std::wstring htmlP(std::wstring_view x);

    inline std::wstring htmlBrP(std::wstring_view x)
        { return htmlBr(htmlP(x)); }

}   // namespace decode
