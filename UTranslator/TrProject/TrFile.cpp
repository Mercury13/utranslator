// My header
#include "TrFile.h"


tf::MobileInfo tf::EnumText::mobileInfo() const
{
    return {
        .escapeInfo = this->escapeInfo,
    };
}



void tf::EnumText::setMobileInfo(const tf::MobileInfo& x)
{
    escapeInfo = x.escapeInfo;
}
