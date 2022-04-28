// My header
#include "TrFile.h"

// Libs
#include "u_Strings.h"


const tf::DummyProto tf::DummyProto::INST;
const tf::IniProto tf::IniProto::INST;
tf::Dummy tf::Dummy::INST;

constinit const tf::FormatProto* const tf::allProtos[I_N] {
    &tf::DummyProto::INST,
    &tf::IniProto::INST
};

const tf::FormatProto* const (&tf::allWorkingProtos)[I_N - 1]
        = reinterpret_cast<const FormatProto* const (&)[I_N - 1]>(allProtos[1]);


///// Loader ///////////////////////////////////////////////////////////////////


void tf::Loader::goToGroupAbs(std::span<const std::u8string_view> groupIds)
{
    goToRoot();
    for (auto v : groupIds)
        goToGroupRel(v);
}


void tf::Loader::goToGroupAbs(std::span<const std::u8string> groupIds)
{
    goToRoot();
    for (auto v : groupIds)
        goToGroupRel(v);
}


///// TextInfo /////////////////////////////////////////////////////////////////

std::u8string tf::TextInfo::joinGroupId(std::u8string_view sep) const
{
    return joinIdToDepth(sep, actualDepth());
}


std::u8string tf::TextInfo::joinTextId(std::u8string_view sep) const
{
    return joinIdToDepth(sep, ids.size());
}

std::u8string tf::TextInfo::joinIdToDepth(std::u8string_view sep, size_t depth) const
{
    std::u8string s;
    for (size_t i = 0; i < depth; ++i) {
        if (i != 0)
            s.append(sep);
        s.append(ids[i]);
    }
    return s;
}


///// DummyProto ///////////////////////////////////////////////////////////////


std::unique_ptr<tf::FileFormat> tf::DummyProto::make() const
    { return std::make_unique<Dummy>(); }


std::u8string_view tf::DummyProto::locDescription() const
{
    return u8"UTranslator will not build this file.";
}

///// IniProto /////////////////////////////////////////////////////////////////

std::unique_ptr<tf::FileFormat> tf::IniProto::make() const
    { return std::make_unique<Ini>(); }

std::u8string_view tf::IniProto::locDescription() const
{
    return u8"Windows settings file often used for localization."
           "<p>[Group1.Group2]<br>"
           "id1=String 1<br>"
           "id2=String 2";
}

///// Ini //////////////////////////////////////////////////////////////////////


tf::UnifiedSets tf::Ini::unifiedSets() const
{
    return {
        .textFormat = this->textFormat,
        .textEscape = this->textEscape,
        .multitier = this->multitier,
    };
}


void tf::Ini::setUnifiedSets(const tf::UnifiedSets& x)
{
    textFormat = x.textFormat;
    textEscape = x.textEscape;
    multitier = x.multitier;
}


std::string tf::Ini::bannedIdChars() const
{
    std::string r = "[]=";
    if (multitier.separator.length() == 1)
        return r += multitier.separator[0];
    return r;
}


void tf::Ini::doExport(
        Walker& walker,
        const std::filesystem::path&,
        const std::filesystem::path& fname)
{
    std::u8string cache;
    std::ofstream os(fname, std::ios::binary);
    if (textFormat.writeBom)
        os << bom::u8;
    auto eol = textFormat.eol();
    bool isInitial = true;
    while (auto& q = walker.nextText()) {
        if (q.groupChanged()) {
            if (!isInitial)
                os << eol;
            os << '[' << str::toSv(q.joinGroupId(multitier.separator)) << ']' << eol;
        }
        os << str::toSv(q.textId()) << '=';
        textEscape.write(os, q.text, cache);
        os << eol;
        isInitial = false;
    }
}


void tf::Ini::save(pugi::xml_node& node) const
    { unifiedSave(node); }


void tf::Ini::load(const pugi::xml_node& node)
    { unifiedLoad(node); }


filedlg::Filter tf::Ini::fileFilter() const
    { return { L"INI files", L"*.ini" }; }
