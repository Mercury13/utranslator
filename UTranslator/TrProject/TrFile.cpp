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
        .lineBreakEscape = this->lineBreakEscape,
        .multitierSeparator = this->separator,
        .writeFlat = this->writeFlat,
        .writeBom = this->writeBom,
    };
}


void tf::Ini::setCommonSets(const tf::CommonSets& x)
{
    lineBreakEscape = x.lineBreakEscape;
    separator = x.multitierSeparator;
    writeFlat = x.writeFlat;
    writeBom = x.writeBom;
}
