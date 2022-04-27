#include "TrDefines.h"

// XML
#include "pugixml.hpp"
#include "u_XmlUtils.h"

// Libs
#include "u_Strings.h"

// Project-local
#include "Decoders.h"


constinit const char* const tr::prjTypeNames[tr::PrjType_N] {
    "original", "full-transl"
};

constinit const tf::LineBreakStyleInfo tf::textLineBreakStyleInfo[TextLineBreakStyle_N] {
    { "lf",   "LF (Unix)",       "\n"   },
    { "crlf", "CR+LF (Windows)", "\r\n" },
};

constinit const tf::LineBreakStyleInfo tf::binaryLineBreakStyleInfo[BinaryLineBreakStyle_N] {
    { "cr",   "CR #13 (Pascal/Windows)", "\r"   },
    { "lf",   "LF #10 (C/Unix)",         "\n"   },
    { "crlf", "CR+LF #13#10",            "\r\n" },
};

constinit const char* const tf::lineBreakEscapeModeNames[LineBreakEscapeMode_N] {
    "banned", "c-cr", "c-lf", "specified" };

constinit const tf::ProtoFilter tf::ProtoFilter::ALL_EXPORTING_AND_NULL {
    .wantedCaps = tf::Fcap::EXPORT,
    .allowEmpty = true
};
constinit const tf::ProtoFilter tf::ProtoFilter::ALL_IMPORTING {
    .wantedCaps = tf::Fcap::IMPORT,
    .allowEmpty = false
};

const tf::TechLoc tf::cSubformatInfo[CSubformat_N] {
    { "bare",   u8"␣Bare␣" },
    { "quoted", u8"\" Quoted \"" },
    { "slashs", u8"\\s for space" },
};


///// TextFormat ///////////////////////////////////////////////////////////////


tf::TextLineBreakStyle tf::TextFormat::parseStyle(std::string_view name)
{
    for (int i = 0; i < TextLineBreakStyle_N; ++i) {
        if (name == textLineBreakStyleInfo[i].techName)
            return static_cast<TextLineBreakStyle>(i);
    }
    return DEFAULT_STYLE;
}


///// TextEscape ///////////////////////////////////////////////////////////////


std::u8string tf::TextEscape::bannedSubstring() const
{
    switch (mode) {
    case LineBreakEscapeMode::BANNED:
        return u8"\n";
    case LineBreakEscapeMode::SPECIFIED_TEXT:
        return specifiedText;
    case LineBreakEscapeMode::C_CR:
    case LineBreakEscapeMode::C_LF:
        return {};
    }
    throw std::logic_error("[TextEscape.bannedSubstring] Strange mode");
}


std::u8string_view tf::TextEscape::escapeSv(
        std::u8string_view x, std::u8string& cache) const
{
    switch (mode) {
    case LineBreakEscapeMode::BANNED:
        return x;
    case LineBreakEscapeMode::SPECIFIED_TEXT:
        return str::replaceSv(x, specifiedText, u8"\n", cache);
    case LineBreakEscapeMode::C_CR:
        return escape::cppSv(x, cache, 'r',
            ecIf<escape::Spaces>(cSubformat == CSubformat::SLASH_SPACE),
            ecIf<Enquote>(cSubformat == CSubformat::QUOTED));
    case LineBreakEscapeMode::C_LF:
        return escape::cppSv(x, cache, 'n',
            ecIf<escape::Spaces>(cSubformat == CSubformat::SLASH_SPACE),
            ecIf<Enquote>(cSubformat == CSubformat::QUOTED));
    }
    throw std::logic_error("[TextEscape.escape] Strange mode");
}


void tf::TextEscape::setSpecifiedText(std::u8string_view x)
{
    if (x.empty()) {
        mode = LineBreakEscapeMode::BANNED;
    } else {
        mode = LineBreakEscapeMode::SPECIFIED_TEXT;
        specifiedText = x;
    }
}


bool tf::TextEscape::isC(LineBreakEscapeMode mode)
{
    switch (mode) {
    case LineBreakEscapeMode::BANNED:
    case LineBreakEscapeMode::SPECIFIED_TEXT:
        return false;
    case LineBreakEscapeMode::C_CR:
    case LineBreakEscapeMode::C_LF:
        return true;
    }
    throw std::logic_error("[TextEscape.isC] Strange mode");
}


///// FormatProto //////////////////////////////////////////////////////////////


bool tf::FormatProto::isWithin(const ProtoFilter& filter) const
{
    auto c = caps();
    return (filter.allowEmpty && !c)
            || (c.haveAll(filter.wantedCaps));
}


///// FileFormat ///////////////////////////////////////////////////////////////


void tf::FileFormat::unifiedSave(pugi::xml_node& node) const
{
    auto sets = unifiedSets();
    auto working = proto().workingSets();

    if (working.have(Usfg::TEXT_FORMAT)) {
        auto nodeFormat = node.append_child("text-format");
        nodeFormat.append_attribute("bom") = sets.textFormat.writeBom;
        nodeFormat.append_attribute("line-break") = sets.textFormat.lineBreakTechName().data();
    }

    if (working.have(Usfg::TEXT_ESCAPE)) {
        auto nodeEscape = node.append_child("text-escape");
        if (sets.textEscape.mode == LineBreakEscapeMode::SPECIFIED_TEXT) {
            nodeEscape.append_attribute("line-break-text") = str::toC(sets.textEscape.specifiedText);
        } else {
            nodeEscape.append_attribute("line-break-mode") =
                    lineBreakEscapeModeNames[static_cast<int>(sets.textEscape.mode)];
            if (sets.textEscape.isC()) {
                nodeEscape.append_attribute("c-subformat") =
                    cSubformatInfo[static_cast<int>(sets.textEscape.cSubformat)].techName.data();
            }
        }
    }

    if (working.have(Usfg::MULTITIER)) {
        auto nodeMulti = node.append_child("multitier");
        nodeMulti.append_attribute("separator") = str::toC(sets.multitier.separator);
    }
}


void tf::FileFormat::unifiedLoad(const pugi::xml_node& node)
{
    auto working = proto().workingSets();
    if (!working)
        return;

    UnifiedSets sets;
    if (working.have(Usfg::TEXT_FORMAT)) {
        if (auto nodeFormat = node.child("text-format")) {
            sets.textFormat.writeBom = nodeFormat.attribute("bom").as_bool();
            sets.textFormat.lineBreakStyle =
                    TextFormat::parseStyle(nodeFormat.attribute("line-break").as_string());
        }
    }

    if (working.have(Usfg::TEXT_ESCAPE)) {
        if (auto nodeEscape = node.child("text-escape")) {
            if (auto attr = nodeEscape.attribute("line-break-text")) {
                sets.textEscape.setSpecifiedText(str::toU8sv(attr.as_string()));
            } else {
                sets.textEscape.mode = parseEnumDef<LineBreakEscapeMode>(
                        nodeEscape.attribute("line-break-mode").as_string(),
                        lineBreakEscapeModeNames,
                        LineBreakEscapeMode::BANNED);
                if (sets.textEscape.mode == LineBreakEscapeMode::SPECIFIED_TEXT)
                    sets.textEscape.mode = LineBreakEscapeMode::BANNED;
                sets.textEscape.cSubformat = parseEnumTechDef(
                        nodeEscape.attribute("c-subformat").as_string(),
                        tf::cSubformatInfo,
                        tf::CSubformat::BARE);
            }
        }
    }

    if (working.have(Usfg::MULTITIER)) {
        if (auto nodeMulti = node.child("multitier")) {
            sets.multitier.separator = str::toU8sv(
                    nodeMulti.attribute("separator").as_string());
        }
    }

    setUnifiedSets(sets);
}
