#include "TrDefines.h"

const char* tr::prjTypeNames[tr::PrjType_N] {
    "original", "full-transl"
};


std::string tf::TextEscape::bannedChars() const
{
    switch (mode) {
    case EscapeMode::NONE:
        return "\r\n";
    case EscapeMode::C_CR:
    case EscapeMode::C_LF:
        return {};
    case EscapeMode::SPECIFIED_CHAR:
        return std::string { specifiedChar };
    }
    throw std::logic_error("[LineBreakEscape.bannedChars] Strange mode");
}
