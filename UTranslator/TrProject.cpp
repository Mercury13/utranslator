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


std::shared_ptr<tr::UiObject> tr::Entity::parent() const
{
    if (auto pg = parentGroup())
        return pg;
    return file();
}


///// Group ////////////////////////////////////////////////////////////////////


std::shared_ptr<tr::UiObject> tr::Group::child(size_t i) const
{
    if (i >= children.size())
        return {};
    return children[i];
}


///// File /////////////////////////////////////////////////////////////////////


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


void tr::Project::addTestOriginal()
{

}
