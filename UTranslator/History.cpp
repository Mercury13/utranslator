// My header
#include "History.h"


//// History ///////////////////////////////////////////////////////////////////


void hist::History::notify()
{
    if (lstn)
        lstn->historyChanged();
}


bool hist::History::silentPush(const std::shared_ptr<Place>& x)
{
    if (!x)
        return false;

    if (!isEmpty()) {
        // x == first?
        if (x->eq(*d.first()))
            return false;

        // The rest
        for (size_t i = 1; i < sz; ++i) {
            auto& v = d[i];
            if (x->eq(*v)) {
                // Move to first
                return silentBump(i);
            }
        }
    }

    // Add new
    if (!isFull())
        ++sz;
    silentBump(sz - 1);
    d.first() = x;
    return true;
}


bool hist::History::silentBump(size_t i)
{
    if (i != 0 && i < sz) {
        auto e1 = d.begin() + i;
        std::rotate(d.begin(), e1, e1 + 1);
        return true;
    } else {
        return false;
    }
}


bool hist::History::push(const std::shared_ptr<Place>& x)
{
    bool b = silentPush(x);
    if (b)
        notify();
    return b;
}


bool hist::History::bump(size_t i)
{
    bool b = silentBump(i);
    if (b)
        notify();
    return b;
}


void hist::History::silentAdd(const std::shared_ptr<Place>& x)
{
    if (!x || size() >= SIZE)
        return;
    d[sz++] = x;
}


bool hist::History::silentClear()
{
    if (sz == 0)
        return false;
    sz = 0;
    d.fill({});
    return true;
}


bool hist::History::clear()
{
    bool b = silentClear();
    if (b)
        notify();
    return b;
}


///// FilePlace ////////////////////////////////////////////////////////////////


bool hist::FilePlace::eq(const Place& x) const
{
    if (auto that = dynamic_cast<const FilePlace*>(&x)) {
        return (*this == *that);
    } else {
        return false;
    }
}


std::wstring hist::FilePlace::shortName() const
{
    return fPath.filename().wstring();
}


std::wstring hist::FilePlace::auxName() const
{
    return fPath.wstring();
}
