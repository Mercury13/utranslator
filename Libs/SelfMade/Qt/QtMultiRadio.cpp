// My header
#include "QtMultiRadio.h"


void UintRadio::setRadio(unsigned index, QAbstractButton* button)
{
    auto sz = index + 1;
    if (sz > v.size())
        v.resize(sz);
    v[index] = button;
}


QAbstractButton* UintRadio::buttonAt(unsigned value) const
{
    if (value >= v.size())
        return nullptr;
    return v[value];
}


void UintRadio::set(unsigned value)
{
    if (auto btn = buttonAt(value)) {
        btn->setChecked(true);
    } else {
        unset();
    }
}


void UintRadio::unset()
{
    for (auto x : v) {
        if (x) {
            x->setChecked(false);
        }
    }
}


unsigned UintRadio::get(unsigned def) const
{
    for (size_t i = 0; i < v.size(); ++i) {
        auto x = v[i];
        if (x && x->isChecked())
            return i;
    }
    return def;
}
