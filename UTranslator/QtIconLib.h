#pragma once

// C++
#include <unordered_map>

// Qt
#include "QPixmap"


class IconLib
{
public:
    /// @todo [future] some modifications of pixmap — IconCache?
    void load(const void* handle, const QString& fname);
    QPixmap get(const void* handle);
private:
    struct Entry {
        QString fname;
        QPixmap pix;
        Entry() = default;
        Entry(const Entry&) = delete;
        Entry(Entry&&) = default;
        Entry& operator = (const Entry&) = delete;
    };
    std::unordered_map<const void*, Entry> d;
};
