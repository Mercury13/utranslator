// My header
#include "TrFile.h"


tf::MobileInfo tf::EnumText::mobileInfo() const
{
    return {
        .lineBreakEscape = this->lineBreakEscape,
    };
}


void tf::EnumText::setMobileInfo(const tf::MobileInfo& x)
{
    lineBreakEscape = x.lineBreakEscape;
}
