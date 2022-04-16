// My header
#include "TrFile.h"

// Libs
#include "u_Strings.h"


const tf::DummyProto tf::DummyProto::INST;
const tf::IniProto tf::IniProto::INST;
tf::Dummy tf::Dummy::INST;

constinit const tf::FormatProto* tf::allProtos[I_N] {
    &tf::DummyProto::INST,
    &tf::IniProto::INST
};


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


///// IniProto /////////////////////////////////////////////////////////////////

std::unique_ptr<tf::FileFormat> tf::IniProto::make() const
    { return std::make_unique<Ini>(); }


///// Ini //////////////////////////////////////////////////////////////////////


tf::UnifiedSets tf::Ini::unifiedSets() const
{
    return {
        .textFormat = this->textFormat,
        .textEscape = this->textEscape,
        .multitierSeparator = this->separator,
    };
}


void tf::Ini::setUnifiedSets(const tf::UnifiedSets& x)
{
    textFormat = x.textFormat;
    textEscape = x.textEscape;
    separator = x.multitierSeparator;
}


std::string tf::Ini::bannedIdChars() const
{
    std::string r = "[]=";
    if (separator.length() == 1)
        return r += separator[0];
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
            os << '[' << str::toSv(q.joinGroupId(separator)) << ']' << eol;
        }
        os << str::toSv(q.textId()) << '='
           << str::toSv(textEscape.escape(q.text, cache)) << eol;
    }
}


void tf::Ini::save(pugi::xml_node&) const
{
    /// @todo [urgent] save!
}
