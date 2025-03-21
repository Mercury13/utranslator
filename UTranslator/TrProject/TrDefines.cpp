#include "TrDefines.h"

// XML
#include "pugixml.hpp"
#include "u_XmlUtils.h"

// Libs
#include "u_Strings.h"

// Project-local
#include "u_Decoders.h"

// C++
#include <array>


const tr::PrjInfo::Transl::Pseudoloc
    tr::PrjInfo::Transl::Pseudoloc::OFF {
        .prefixSuffixMode = tr::PrefixSuffixMode::OFF  },
    tr::PrjInfo::Transl::Pseudoloc::DFLT {
        .prefixSuffixMode = tr::PrefixSuffixMode::DFLT };


constinit const ec::Array<const char*, tr::PrjType> tr::prjTypeNames {
    "original", "full-transl" };

constinit const ec::Array<tf::LineBreakStyleInfo, tf::TextLineBreakStyle> tf::textLineBreakStyleInfo {
    ec::ARRAY_INIT,
    std::to_array<tf::LineBreakStyleInfo>({
        { "lf",   "LF (Unix)",       "\n"   },
        { "crlf", "CR+LF (Windows)", "\r\n" },
    })
};

//constinit const ec::Array<tf::LineBreakStyleInfo, tf::BinaryLineBreakStyle> tf::binaryLineBreakStyleInfo {
//    tf::LineBreakStyleInfo { "cr",   "CR #13 (Pascal/Windows)", "\r"   },
//    tf::LineBreakStyleInfo { "lf",   "LF #10 (C/Unix)",         "\n"   },
//    tf::LineBreakStyleInfo { "crlf", "CR+LF #13#10",            "\r\n" },
//};

constinit const ec::Array<std::string_view, escape::LineBreakMode> tf::lineBreakEscapeModeNames {
    "banned", "c-cr", "c-lf", "specified" };

constinit const tf::ProtoFilter tf::ProtoFilter::ALL_EXPORTING_AND_NULL {
    .wantedCaps = tf::Fcap::EXPORT,
    .allowEmpty = true
};
constinit const tf::ProtoFilter tf::ProtoFilter::ALL_IMPORTING {
    .wantedCaps = tf::Fcap::IMPORT,
    .allowEmpty = false
};

const ec::Array<tf::TechLoc, escape::SpaceMode> tf::spaceEscapeModeInfo {
    ec::ARRAY_INIT,
    std::to_array<TechLoc>({
        { "bare",    u8"␣Bare␣"            },
        { "delim",   u8"␣Delimiter␣|"      },
        { "quoted",  u8"\" Quoted \""       },
        { "slashs",  u8"\\s (C-like only!)" },
    })
};

constinit const char* const tf::textOwnerNames[TextOwner_N] {
    "editor", "me" };


///// TextFormat ///////////////////////////////////////////////////////////////


tf::TextLineBreakStyle tf::TextFormat::parseStyle(std::string_view name)
{
    return textLineBreakStyleInfo.findIfDef(DEFAULT_STYLE,
            [name](auto& x) { return x.techName == name; });
}

///// FormatProto //////////////////////////////////////////////////////////////


bool tf::FormatProto::isWithin(const ProtoFilter& filter) const
{
    auto c = caps();
    return (filter.allowEmpty && !c)
            || (c.haveAll(filter.wantedCaps));
}


///// PrjInfo //////////////////////////////////////////////////////////////////


bool tr::PrjInfo::canEditOriginal(PrjType type)
{
    switch (type) {
    case PrjType::ORIGINAL:
        return true;
    case PrjType::FULL_TRANSL:
        return false;
    }
    throw std::logic_error("[canEditOriginal] Strange project type");
}


bool tr::PrjInfo::canAddFiles(PrjType type)
{
    switch (type) {
    case PrjType::ORIGINAL:
        return true;
    case PrjType::FULL_TRANSL:
        return false;
    }
    throw std::logic_error("[canAddFiles] Strange project type");
}


bool tr::PrjInfo::isTranslation(PrjType type)
{
    switch (type) {
    case PrjType::ORIGINAL:
        return false;
    case PrjType::FULL_TRANSL:
        return true;
    }
    throw std::logic_error("[isTranslation] Strange project type");
}


bool tr::PrjInfo::hasOriginalPath(PrjType type)
{
    switch (type) {
    case PrjType::ORIGINAL:
        return false;
    case PrjType::FULL_TRANSL:
        return true;
    }
    throw std::logic_error("[hasOriginalPath] Strange project type");
}


bool tr::PrjInfo::canHaveReference(PrjType type)
{
    switch (type) {
    case PrjType::ORIGINAL:
        return false;
    case PrjType::FULL_TRANSL:
        return true;
    }
    throw std::logic_error("[canHaveReference] Strange project type");
}


void tr::PrjInfo::switchOriginalAndTranslation(const std::filesystem::path& path)
{
    std::swap(orig.lang, transl.lang);
    orig.absPath = path;
}


void tr::PrjInfo::switchToOriginal(WalkChannel channel)
{
    type = PrjType::ORIGINAL;
    switch (channel) {
    case WalkChannel::ORIGINAL:
        break;
    case WalkChannel::TRANSLATION:
        orig.lang = transl.lang;
        break;
    }
    orig.absPath.clear();
    ref.clear();
    transl.clear();
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
        if (sets.textEscape.lineBreak == escape::LineBreakMode::SPECIFIED_TEXT) {
            nodeEscape.append_attribute("line-break-text") = str::toC(sets.textEscape.lineBreakText);
        } else {
            // OK s_v.data here, constinit
            nodeEscape.append_attribute("line-break-mode") =
                    lineBreakEscapeModeNames[sets.textEscape.lineBreak].data();
        }
        if (sets.textEscape.space == escape::SpaceMode::DELIMITED) {
            nodeEscape.append_attribute("space-delimiter") = str::toC(sets.textEscape.spaceDelimiter);
        } else {
            // OK s_v.data here, constinit
            nodeEscape.append_attribute("space-escape") =
                spaceEscapeModeInfo[sets.textEscape.space].techName.data();
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
                sets.textEscape.setLineBreakText(str::toU8sv(attr.as_string()));
            } else {
                sets.textEscape.lineBreak = lineBreakEscapeModeNames.findDef(
                        nodeEscape.attribute("line-break-mode").as_string(),
                        escape::LineBreakMode::BANNED);
                if (sets.textEscape.lineBreak == escape::LineBreakMode::SPECIFIED_TEXT)
                    sets.textEscape.lineBreak = escape::LineBreakMode::BANNED;
            }
            if (auto attr = nodeEscape.attribute("space-delimiter")) {
                sets.textEscape.setSpaceDelimiter(str::toU8sv(attr.as_string()));
            } else {
                sets.textEscape.space = parseEnumTechDef(
                        nodeEscape.attribute("space-escape").as_string(),
                        tf::spaceEscapeModeInfo.cArray(),
                        escape::SpaceMode::BARE);
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
