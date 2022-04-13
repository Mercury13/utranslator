#include "TrDefines.h"

const char* tr::prjTypeNames[tr::PrjType_N] {
    "original", "full-transl"
};


std::string tf::TextEscape::bannedSubstring() const
{
    switch (mode) {
    case EscapeMode::NONE:
        return "\n";
    case EscapeMode::SPECIFIED_TEXT:
        return specifiedText;
    case EscapeMode::C_CR:
    case EscapeMode::C_LF:
        return {};
    }
    throw std::logic_error("[LineBreakEscape.bannedChars] Strange mode");
}
