// My header
#include "TrFile.h"

// Libs
#include "u_Strings.h"


///// TextInfo /////////////////////////////////////////////////////////////////


std::u8string tf::TextInfo::joinGroupId(char c) const
{
    size_t n = ids.size();
    if (n == 0)
        return {};
    --n;
    std::u8string s;
    for (size_t i = 0; i < n; ++i) {
        if (!s.empty())
            s.push_back(c);
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


const tf::DummyProto tf::DummyProto::INST;

std::unique_ptr<tf::FileFormat> tf::DummyProto::make() const
    { return std::make_unique<Dummy>(); }


///// IniProto /////////////////////////////////////////////////////////////////

const tf::IniProto tf::IniProto::INST;

std::unique_ptr<tf::FileFormat> tf::IniProto::make() const
    { return std::make_unique<Ini>(); }


///// Ini //////////////////////////////////////////////////////////////////////


tf::CommonSets tf::Ini::commonSets() const
{
    return {
        .textFormat = this->textFormat,
        .textEscape = this->textEscape,
        .multitierSeparator = this->separator,
        .writeFlat = this->writeFlat,
    };
}


void tf::Ini::setCommonSets(const tf::CommonSets& x)
{
    textFormat = x.textFormat;
    textEscape = x.textEscape;
    separator = x.multitierSeparator;
    writeFlat = x.writeFlat;
}


std::string tf::Ini::bannedIdChars() const
{
    return std::string("[]=") + separator;
}


void tf::Ini::doExport(
        Walker& walker,
        const std::filesystem::path& fname)
{
    std::ofstream os(fname);
    while (auto& q = walker.nextText()) {
        if (q.groupChanged()) {
            os << '[' << str::toSv(q.joinGroupId(separator)) << ']' << std::endl;
        }
        /// @todo [urgent] escape!!
        os << str::toSv(q.textId()) << '=' << str::toSv(q.text) << std::endl;
    }
}
