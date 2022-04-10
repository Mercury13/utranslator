// My header
#include "History.h"

// Project-local
#include "u_Strings.h"

// XML
#include "pugixml.hpp"


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


bool hist::History::firstIs(const Place& x)
{
    return (!isEmpty() &&
            (d.first().get() == &x      // they point to the same object
             || d.first()->eq(x)));     // they are equal
}


void hist::History::save(
        pugi::xml_node& root, const char* name) const
{
    auto node = root.append_child(name);
    for (auto& v : *this) {
        v->save(node);
    }
}


///// FilePlace ////////////////////////////////////////////////////////////////

namespace {

    std::filesystem::path toCanonicalNe(const std::filesystem::path& x)
    {
        try {
            return std::filesystem::canonical(x);
        } catch (const std::bad_alloc&) {
            throw;
        } catch (...) {
            return x;
        }
    }

}   // anon namespace


hist::FilePlace::FilePlace(const std::filesystem::path& aPath)
    : fPath(toCanonicalNe(aPath)) {}


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


void hist::FilePlace::save(pugi::xml_node& root) const
{
    root.append_child("file")
        .append_attribute("name") = str::toC(fPath.u8string());
}
