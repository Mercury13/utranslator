#include "TrDefines.h"

#include "u_Strings.h"

#include "Decoders.h"


const char* tr::prjTypeNames[tr::PrjType_N] {
    "original", "full-transl"
};


std::u8string tf::TextEscape::bannedSubstring() const
{
    switch (mode) {
    case EscapeMode::NONE:
        return u8"\n";
    case EscapeMode::SPECIFIED_TEXT:
        return specifiedText;
    case EscapeMode::C_CR:
    case EscapeMode::C_LF:
        return {};
    }
    throw std::logic_error("[LineBreakEscape.bannedChars] Strange mode");
}


std::u8string_view tf::TextEscape::escape(
        std::u8string_view x, std::u8string& cache) const
{
    switch (mode) {
    case EscapeMode::NONE:
        return x;
    case EscapeMode::SPECIFIED_TEXT:
        return str::replaceSv(x, specifiedText, u8"\n", cache);
    case EscapeMode::C_CR:
        return escape::cpp(x, 'r', cache);
    case EscapeMode::C_LF:
        return escape::cpp(x, 'n', cache);
    }
}
