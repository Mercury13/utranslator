#include "TrDefines.h"

// XML
#include "pugixml.hpp"

const tr::PrjInfo::Transl::Pseudoloc
    tr::PrjInfo::Transl::Pseudoloc::OFF {
        .prefixSuffixMode = tr::PrefixSuffixMode::OFF  },
    tr::PrjInfo::Transl::Pseudoloc::DFLT {
        .prefixSuffixMode = tr::PrefixSuffixMode::DFLT };


constinit const ec::Array<const char*, tr::PrjType> tr::prjTypeNames {
    "original", "full-transl" };

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
