#include "TrProject.h"

#include <bit>

// Libs
#include "u_Strings.h"

///// CanaryObject /////////////////////////////////////////////////////////////

tr::CanaryObject::CanaryObject() : canary(goodCanary()) {}


uint32_t tr::CanaryObject::goodCanary() const
{
    static constexpr size_t ALIGNMENT = alignof (uintptr_t);
    static constexpr size_t N_BITS = std::countr_zero(ALIGNMENT);
    static constexpr uint32_t SCRAMBLE = 0xC21330A5;
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) >> N_BITS) ^ SCRAMBLE;
}


void tr::CanaryObject::checkCanary() const
{
    if (canary != goodCanary())
        throw std::logic_error("[UiObject.checkCanary] Canary is dead, pointer to nowhere!");
}


tr::CanaryObject::~CanaryObject()
{
    canary = 0;
}


///// UiObject /////////////////////////////////////////////////////////////////


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


void tr::UiObject::recursiveUnmodify()
{
    cache.mod.clear();
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        ch->recursiveUnmodify();
    }
}



void tr::UiObject::doModify(Mch ch)
{
    cache.mod.set(ch);
    if (auto p = project())
        p->modify();
}



bool tr::UiObject::setOriginal(std::u8string_view x, tr::Modify wantModify)
{
    if (auto t = translatable()) {
        if (t->original != x) {
            t->original = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setTranslation(
        std::optional<std::u8string_view> x, tr::Modify wantModify)
{
    if (auto t = translatable()) {
        if (t->translation != x) {
            t->translation = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::TRANSL);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setAuthorsComment(std::u8string_view x, tr::Modify wantModify)
{
    if (auto c = comments()) {
        if (c->authors != x) {
            c->authors = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::COMMENT);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setTranslatorsComment(std::u8string_view x, tr::Modify wantModify)
{
    if (auto c = comments()) {
        if (c->translators != x) {
            c->translators = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::COMMENT);
            }
            return true;
        }
    }
    return false;
}


std::u8string tr::UiObject::makeId(
        std::u8string_view prefix,
        std::u8string_view suffix) const
{
    auto nc = nChildren();
    size_t newIndex = 0;
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        auto sNumber = str::remainderSv(ch->id, prefix, suffix);
        size_t num = 0;
        auto chars = str::fromChars(sNumber, num);
        if (chars.ec == std::errc() && num >= newIndex) {
            newIndex = num + 1;
        }
    }
    char buf[30];
    auto sIndex = str::toCharsU8(buf, newIndex);

    std::u8string s;
    s.append(prefix);
    s.append(sIndex);
    s.append(suffix);
    return s;
}


std::shared_ptr<tr::Entity> tr::UiObject::extract()
{
    auto pnt = parent();
    // No parent?
    if (!pnt)
        return {};

    auto nc = pnt->nChildren();
    // Initial cache lookup
    if (static_cast<size_t>(cache.index) < nc
            && pnt->child(cache.index).get() == this) {
        return pnt->extractChild(cache.index);
    }

    // Search by cache
    for (size_t i = 0; i < nc; ++i) {
        if (pnt->child(i).get() == this) {
            return pnt->extractChild(cache.index);
        }
    }
    return {};
}


///// Entity ///////////////////////////////////////////////////////////////////


bool tr::Entity::setId(std::u8string_view x, tr::Modify wantModify)
{
    if (id != x) {
        id = x;
        if (wantModify != Modify::NO) {
            doModify(Mch::ID);
        }
        return true;
    } else {
        return false;
    }
}

size_t tr::UiObject::nTexts() const
{
    auto nc = nChildren();
    size_t r = 0;
    for (size_t i = 0; i < nc; ++i) {
        r += child(i)->nTexts();
    }
    return r;
}

///// VirtualGroup /////////////////////////////////////////////////////////////


std::shared_ptr<tr::Entity> tr::VirtualGroup::child(size_t i) const
{
    if (i >= children.size())
        return {};
    return children[i];
}


std::shared_ptr<tr::Text> tr::VirtualGroup::addText(
        std::u8string id, std::u8string original,
        Modify wantModify)
{
    auto index = nChildren();
    auto r = std::make_shared<Text>(fSelf, index, PassKey{});
    r->id = std::move(id);
    r->tr.original = std::move(original);
    if (wantModify != Modify::NO) {
        doModify(Mch::ID);
    }
    children.push_back(r);
    return r;
}


std::shared_ptr<tr::Group> tr::VirtualGroup::addGroup(
        std::u8string id, Modify wantModify)
{
    auto index = nChildren();
    auto r = std::make_shared<Group>(fSelf.lock(), index, PassKey{});
    r->fSelf = r;
    r->id = std::move(id);
    if (wantModify != Modify::NO) {
        doModify(Mch::ID);
    }
    children.push_back(r);
    return r;
}


std::shared_ptr<tr::Project> tr::VirtualGroup::project()
{
    if (auto f = file())
        return f->project();
    return nullptr;
}


std::shared_ptr<tr::Entity> tr::VirtualGroup::extractChild(size_t i)
{
    if (i >= children.size())
        return {};
    auto r = children[i];
    children.erase(children.begin() + i);
    recache();
    return r;
}


///// Group ////////////////////////////////////////////////////////////////////


tr::Group::Group(
        const std::shared_ptr<VirtualGroup>& aParent,
        size_t aIndex, const PassKey&)
    : fParentGroup(aParent)
{
    if (!aParent)
        throw std::invalid_argument("[Group.ctor] parent should not be null!");
    fFile = aParent->file();
    cache.index = aIndex;
}


///// Text /////////////////////////////////////////////////////////////////////


tr::Text::Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const PassKey&)
    : fParentGroup(std::move(aParent))
{
    cache.index = aIndex;
}


std::shared_ptr<tr::File> tr::Text::file()
{
    if (auto lk = fParentGroup.lock())
        return lk->file();
    return nullptr;
}


std::shared_ptr<tr::Project> tr::Text::project()
{
    if (auto f = file())
        return f->project();
    return nullptr;
}


///// File /////////////////////////////////////////////////////////////////////


tr::File::File(
        std::weak_ptr<tr::Project> aProject,
        size_t aIndex, const PassKey&)
    : fProject(std::move(aProject))
{
    cache.index = aIndex;
}


std::shared_ptr<tr::UiObject> tr::File::parent() const
    { return fProject.lock(); }


///// Project //////////////////////////////////////////////////////////////////


std::shared_ptr<tr::Entity> tr::Project::child(size_t i) const
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
    if (auto lk = fSelf.lock())
        return lk;
    // Aliasing ctor
    return std::shared_ptr<Project>(std::shared_ptr<int>{}, this);
}


std::shared_ptr<tr::File> tr::Project::addFile(
        std::u8string_view name, Modify wantModify)

{
    auto index = nChildren();
    auto r = std::make_shared<File>(self(), index, PassKey{});
    r->fSelf = r;
    r->id = name;
    if (wantModify != Modify::NO) {
        doModify(Mch::ID);
    }
    files.push_back(r);
    return r;
}


void tr::Project::addTestOriginal()
{
    auto file = addFile(u8"vowel.txt", Modify::NO);
        file->addText(u8"A", u8"Alpha", Modify::NO);
        file->addText(u8"E", u8"Echo", Modify::NO);
        file->addText(u8"I", u8"India", Modify::NO);
        file->addText(u8"O", u8"Oscar", Modify::NO);
        file->addText(u8"U", u8"Uniform", Modify::NO);
        auto group = file->addGroup(u8"Specials", Modify::NO);
            group->addText(u8"Y", u8"Yankee", Modify::NO);
    file = addFile(u8"consonant.txt", Modify::NO);
        file->addText(u8"B", u8"Bravo", Modify::NO);
        file->addText(u8"C", u8"Charlie", Modify::NO);
        file->addText(u8"D", u8"Delta", Modify::NO);
}


void tr::Project::doShare(const std::shared_ptr<Project>& x)
{
    if (x && x.get() != this)
        throw std::logic_error("[Project.doShare] x should be null, or point to the project itself!");
    fSelf = x;
}


std::shared_ptr<tr::Entity> tr::Project::extractChild(size_t i)
{
    if (i >= files.size())
        return {};
    auto r = files[i];
    files.erase(files.begin() + i);
    recache();
    return r;
}


bool tr::Project::unmodify()
{
    bool r = SimpleModifiable::unmodify();
    if (r) {
        recursiveUnmodify();
    }
    return r;
}
