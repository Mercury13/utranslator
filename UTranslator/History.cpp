// My header
#include "History.h"

// Project-local
#include "u_Strings.h"

// XML
#include "pugixml.hpp"

using namespace std::string_view_literals;


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


void hist::History::pushFile(std::filesystem::path fname)
{
    push(std::make_shared<FilePlace>(std::move(fname)));
}


bool hist::History::bump(size_t i)
{
    bool b = silentBump(i);
    if (b)
        notify();
    return b;
}


bool hist::History::silentAdd(std::shared_ptr<Place> x)
{
    if (!x || size() >= SIZE)
        return false;
    d[sz++] = std::move(x);
    return true;
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


void hist::History::silentLoad(pugi::xml_node& node,
          std::initializer_list<EvLoadPlace> loadPlaces)
{
    clear();
    for (auto child : node.children()) {
        for (auto loader : loadPlaces) {
            if (auto up = loader(child)) {
                silentAdd(std::move(up));
                if (isFull()) {
                    return;
                } else {
                    break;
                }
            }
        }
        // break here
    }
}


void hist::History::load(pugi::xml_node& node,
          std::initializer_list<EvLoadPlace> loadPlaces)
{
    silentLoad(node, loadPlaces);
    notify();
}


hist::History::Sp hist::History::operator [] (size_t i) const noexcept
{
    if (i >= sz)
        return {};
    return d[i];
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

hist::FilePlace::FilePlace(std::filesystem::path&& aPath)
    : fPath(std::move(aPath))
{
    if (!fPath.is_absolute()) {
        fPath = toCanonicalNe(fPath);
    }
}


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


std::unique_ptr<hist::FilePlace> hist::FilePlace::tryLoadSpec(const pugi::xml_node& node)
{
    if (node.name() != "file"sv)
        return {};
    const char* szPath = node.attribute("name").as_string();
    std::filesystem::path path { str::toU8sv(szPath) };
    if (!path.is_absolute())
        return {};
    return std::make_unique<FilePlace>(std::move(path));
}
