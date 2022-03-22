#include "TrProject.h"

#include <bit>

///// UiObject /////////////////////////////////////////////////////////////////

tr::UiObject::UiObject()
{
    canary = goodCanary();
}


tr::UiObject::~UiObject()
{
    canary = 0;
}


uint32_t tr::UiObject::goodCanary() const
{
    static constexpr size_t ALIGNMENT = alignof (uintptr_t);
    static constexpr size_t N_BITS = std::countr_zero(ALIGNMENT);
    static constexpr uint32_t SCRAMBLE = 0xC21330A5;
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) >> N_BITS) ^ SCRAMBLE;
}


void tr::UiObject::checkCanary() const
{
    if (canary != goodCanary())
        throw std::logic_error("[UiObject.checkCanary] Canary is dead, pointer to nowhere!");
}


void tr::UiObject::recache()
{
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        child(i)->cache.index = i;
    }
}


void tr::UiObject::recursiveRecache()
{
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        ch->cache.index = i;
        ch->recursiveRecache();
    }
}


///// Entity ///////////////////////////////////////////////////////////////////


///// VirtualGroup /////////////////////////////////////////////////////////////


std::shared_ptr<tr::UiObject> tr::VirtualGroup::child(size_t i) const
{
    if (i >= children.size())
        return {};
    return children[i];
}


std::shared_ptr<tr::Text> tr::VirtualGroup::addText(
        std::u8string id, std::u8string original)
{
    auto index = nChildren();
    auto r = std::make_shared<Text>(self, index, Key{});
    r->id = std::move(id);
    r->original = std::move(original);
    children.push_back(r);
    return r;
}


///// Text /////////////////////////////////////////////////////////////////////


tr::Text::Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const Key&)
    : fParentGroup(std::move(aParent))
{
    cache.index = aIndex;
}


///// File /////////////////////////////////////////////////////////////////////


tr::File::File(
        std::weak_ptr<tr::Project> aProject,
        size_t aIndex, const Key&)
    : fProject(std::move(aProject))
{
    cache.index = aIndex;
}


std::shared_ptr<tr::UiObject> tr::File::parent() const
    { return fProject.lock(); }


///// Project //////////////////////////////////////////////////////////////////


std::shared_ptr<tr::UiObject> tr::Project::child(size_t i) const
{
    if (i >= files.size())
        return {};
    return files[i];
}


void tr::Project::clear()
{
    *this = Project();
}


std::shared_ptr<tr::Project> tr::Project::self()
{
    try {
        return shared_from_this();
    } catch (const std::bad_weak_ptr&) {
        // Uncontrolled by shared — use aliasing ctor
        return std::shared_ptr<Project>(std::shared_ptr<int>{}, this);
    }
}


std::shared_ptr<tr::File> tr::Project::addFile()
{
    auto index = nChildren();
    auto r = std::make_shared<File>(self(), index, Key{});
    r->self = r;
    files.push_back(r);
    return r;
}


std::shared_ptr<tr::File> tr::Project::addFile(std::u8string_view name)
{
    auto r = addFile();
    r->id = name;
    return r;
}


void tr::Project::addTestOriginal()
{
    auto file = addFile(u8"vowel.txt");
        file->addText(u8"A", u8"Alpha");
        file->addText(u8"E", u8"Echo");
        file->addText(u8"I", u8"India");
        file->addText(u8"O", u8"Oscar");
        file->addText(u8"U", u8"Uniform");
    file = addFile(u8"consonant.txt");
        file->addText(u8"B", u8"Bravo");
        file->addText(u8"C", u8"Charlie");
        file->addText(u8"D", u8"Delta");
}
