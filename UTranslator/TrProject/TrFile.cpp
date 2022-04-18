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


///// TextInfo /////////////////////////////////////////////////////////////////


std::u8string tf::TextInfo::joinGroupId(std::u8string_view sep) const
{
    size_t n = ids.size();
    if (n == 0)
        return {};
    --n;
    std::u8string s;
    for (size_t i = 0; i < n; ++i) {
        if (!s.empty())
            s.append(sep);
        s.append(ids[i]);
    }
    return s;
}


//tf::MobileInfo tf::EnumText::mobileInfo() const
//{
//    return {
//        .lineBreakEscape = this->lineBreakEscape,
//    };
//}


//void tf::EnumText::setMobileInfo(const tf::MobileInfo& x)
//{
//    lineBreakEscape = x.lineBreakEscape;
//}

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
        const std::filesystem::path& fname)
{
    std::u8string cache;
    std::ofstream os(fname, std::ios::binary);
    auto eol = textFormat.eol();
    while (auto& q = walker.nextText()) {
        if (q.groupChanged()) {
            os << '[' << str::toSv(q.joinGroupId(multitier.separator)) << ']' << eol;
        }
        os << str::toSv(q.textId()) << '='
           << str::toSv(textEscape.escapeSv(q.text, cache)) << eol;
    }
}


void tf::Ini::save(pugi::xml_node& node) const
    { unifiedSave(node); }


void tf::Ini::load(const pugi::xml_node& node)
    { unifiedLoad(node); }
