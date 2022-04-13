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


///// Ini //////////////////////////////////////////////////////////////////////


tf::CommonSets tf::Ini::commonSets() const
{
    return {
        .textEscape = this->textEscape,
        .multitierSeparator = this->separator,
        .writeFlat = this->writeFlat,
    };
}


void tf::Ini::setCommonSets(const tf::CommonSets& x)
{
    textEscape = x.textEscape;
    separator = x.multitierSeparator;
    writeFlat = x.writeFlat;
}
