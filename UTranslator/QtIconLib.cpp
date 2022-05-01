#include "QtIconLib.h"


void IconLib::load(const void* handle, const QString& fname)
{
    auto& entry = d[handle];
    entry.fname = fname;
}


QPixmap IconLib::get(const void* handle)
{
    auto it = d.find(handle);
    if (it == d.end())
        return {};
    auto& entry = it->second;
    if (entry.pix.isNull()) {
        entry.pix.load(entry.fname);
    }
    return entry.pix;
}
