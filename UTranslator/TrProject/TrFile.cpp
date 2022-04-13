// My header
#include "TrFile.h"


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


///// IniProto /////////////////////////////////////////////////////////////////

const tf::IniProto tf::IniProto::INST;

std::unique_ptr<tf::FileFormat> tf::IniProto::make() const
{
    return std::make_unique<Ini>();
}


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

}
