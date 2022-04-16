#include "TrDefines.h"

#include "u_Strings.h"

#include "Decoders.h"


const char* tr::prjTypeNames[tr::PrjType_N] {
    "original", "full-transl"
};


constinit tf::LineBreakStyleInfo tf::lineBreakStyleInfo[LineBreakStyle_N] {
    { "cr", "\r" },
    { "lf", "\n" },
    { "crlf", "\r\n" },
};


///// TextEscape ///////////////////////////////////////////////////////////////


std::u8string tf::TextEscape::bannedSubstring() const
{
    switch (mode) {
    case LineBreakEscapeMode::NONE:
        return u8"\n";
    case LineBreakEscapeMode::SPECIFIED_TEXT:
        return specifiedText;
    case LineBreakEscapeMode::C_CR:
    case LineBreakEscapeMode::C_LF:
        return {};
    }
    throw std::logic_error("[TextEscape.bannedSubstring] Strange mode");
}


std::u8string_view tf::TextEscape::escape(
        std::u8string_view x, std::u8string& cache) const
{
    switch (mode) {
    case LineBreakEscapeMode::NONE:
        return x;
    case LineBreakEscapeMode::SPECIFIED_TEXT:
        return str::replaceSv(x, specifiedText, u8"\n", cache);
    case LineBreakEscapeMode::C_CR:
        return escape::cppSv(x, 'r', cache);
    case LineBreakEscapeMode::C_LF:
        return escape::cppSv(x, 'n', cache);
    }
    throw std::logic_error("[TextEscape.escape] Strange mode");
}


///// FormatProto //////////////////////////////////////////////////////////////


bool tf::FormatProto::isWithin(const ProtoFilter& filter) const
{
    auto c = caps();
    return (filter.allowEmpty && !c)
            || (c.haveAll(filter.wantedCaps));
}
