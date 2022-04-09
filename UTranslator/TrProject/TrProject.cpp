#include "TrProject.h"

// C++
#include <stdexcept>
#include <bit>

// Libs
#include "u_Strings.h"

// Pugixml
#include "pugixml.hpp"


///// XML utils ////////////////////////////////////////////////////////////////

namespace {

    template <class T>
    inline T rq(T&& val, const char* errmsg)
    {
        if (!val) throw std::logic_error(errmsg);
        return std::forward<T>(val);
    }

    auto rqChild(pugi::xml_node node, const char* name)
    {
        auto child = node.child(name);
        if (!child)
            throw std::logic_error(
                    std::string("Tag <") + node.name() + "> needs child <"
                    + name + ">");
        return child;
    }

    /// Need non-null attribute
    auto rqAttr(pugi::xml_node node, const char* name)
    {
        auto attr = node.attribute(name);
        if (!attr)
            throw std::logic_error(
                    std::string("Tag <") + node.name() + "> needs attribute <"
                    + name + ">");
        return attr;
    }

    int parseEnumIntDef(const char* text, int n, const char* names[], int def)
    {
        if (!text)
            return def;
        for (int i = 0; i < n; ++i) {
            if (strcmp(text, names[i]) == 0)
                return i;
        }
        return def;
    }

    inline std::string toStr(const char* text)
        { return text ? std::string{text} : std::string{}; }

    [[maybe_unused]] int parseEnumIntRq(const char* text, int n, const char* names[])
    {
        auto val = parseEnumIntDef(text, n, names, -1);
        if (val < 0)
            throw std::logic_error("Unknown name <" + toStr(text) + ">");
        return val;
    }

    template <class Ec, size_t N> requires std::is_enum_v<Ec>
    inline Ec parseEnumDef(const char* text, const char* (&names)[N], Ec def)
        { return static_cast<Ec>(parseEnumIntDef(
                text, N, names, static_cast<int>(def))); }

    template <class Ec, size_t N> requires std::is_enum_v<Ec>
    inline Ec parseEnumRq(const char* text, const char* (&names)[N])
        { return static_cast<Ec>(parseEnumIntRq(text, N, names)); }

}   // anon namespace

///// WrCache //////////////////////////////////////////////////////////////////


void tr::WrCache::ensureU8(size_t length)
{
    length += 8;
    if (u8.length() < length)
        u8.resize(std::max<size_t>(64, length * 3 / 2));
}


const char8_t* tr::WrCache::nts(const char8_t* beg, const char8_t* end)
{
    auto len = end - beg;
    if (len == 0)
        return u8"";
    ensureU8(end - beg);
    auto e = std::copy(beg, end, u8.begin());
    *e = 0;
    return u8.data();
}


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


bool tr::UiObject::setIdless(bool x, tr::Modify wantModify)
{
    if (auto fi = ownFileInfo()) {
        if (fi->isIdless != x) {
            fi->isIdless = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
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


const tr::FileInfo* tr::UiObject::inheritedFileInfo() const
{
    if (auto f = file()) {
        if (auto fi = f->ownFileInfo()) {
            return fi;
        }
    }
    return nullptr;
}


std::u8string tr::UiObject::makeTextId(const IdLib& idlib) const
{
    if (auto fi = inheritedFileInfo(); fi && !fi->isIdless) {
        return makeId(idlib.textPrefix, {});
    }
    return {};
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


void tr::UiObject::addStatsRecursive(Stats& x, bool includeSelf) const
{
    addStats(x, includeSelf);
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        child(i)->addStatsRecursive(x, true);
    }
}


tr::Stats tr::UiObject::stats(bool includeSelf) const
{
    Stats r;
    addStatsRecursive(r, includeSelf);
    return r;
}

namespace {

    /// Write text in tag
    /// @param root   an upper element
    /// @param name   tag name
    /// @param text   text itself
    void writeTextInTag(
            pugi::xml_node root,
            const char* name,
            const std::u8string& text,
            tr::WrCache& cache)
    {
        const char8_t* data = text.data();
        const char8_t* end = data + text.length();

        auto node = root.append_child(name);
        // Initial lines
        while (true) {
            // Find CR
            auto p = std::find(data, end, '\n');
            if (p == end)
                break;
            // Append text and BR
            auto tx = node.append_child(pugi::node_pcdata);
                tx.set_value(cache.ntsC(data, p));
            node.append_child("br");
            data = p + 1;
        }
        // Final line
        auto tx = node.append_child(pugi::node_pcdata);
            tx.set_value(str::toC(data));
    }

    /// Write text in tag if the text is not empty (for comments)
    void writeTextInTagIf(
            pugi::xml_node root,
            const char* name,
            const std::u8string& text,
            tr::WrCache& cache)
    {
        if (!text.empty())
            writeTextInTag(root, name, text, cache);
    }

    /// Write text in tag if the optional is not empty (for known text, translation)
    void writeTextInTagOpt(
            pugi::xml_node root,
            const char* name,
            const std::optional<std::u8string>& text,
            tr::WrCache& cache)
    {
        if (text.has_value())
            writeTextInTag(root, name, *text, cache);
    }

    std::u8string parseTextInTag(pugi::xml_node tag)
    {
        std::u8string r;
        for (auto v : tag.children()) {
            switch (v.type()) {
            case pugi::node_pcdata:
            case pugi::node_cdata:
                r += str::toU8sv(v.value());
                break;
            case pugi::node_element:
                if (strcmp(v.name(), "br") == 0)
                    r += '\n';
                break;
            default: ;
            }
        }
        return r;
    }

    std::u8string readTextInTag(
            pugi::xml_node root, const char* name)
    {
        if (auto tag = root.child(name))
            return parseTextInTag(tag);
        return {};
    }

    std::optional<std::u8string> readTextInTagOpt(
            pugi::xml_node root, const char* name)
    {
        if (auto tag = root.child(name))
            return parseTextInTag(tag);
        return std::nullopt;
    }

}   // anon namespace


void tr::Entity::writeAuthorsComment(
        pugi::xml_node& node, WrCache& c) const
{
    writeTextInTagIf(node, "au-cmt", comm.authors, c);
}

void tr::Entity::readAuthorsComment(const pugi::xml_node& node)
{
    comm.authors = readTextInTag(node, "au-cmt");
}

void tr::Entity::writeTranslatorsComment(
        pugi::xml_node& node, WrCache& c) const
{
    switch (c.info.type) {
    case PrjType::ORIGINAL:
        break;
    case PrjType::FULL_TRANSL:
        writeTextInTagIf(node, "tr-cmt", comm.translators, c);
        break;
    }
}

void tr::Entity::readTranslatorsComment(const pugi::xml_node& node, const PrjInfo& info)
{
    switch (info.type) {
    case PrjType::ORIGINAL:
        break;
    case PrjType::FULL_TRANSL:
        comm.translators = readTextInTag(node, "tr-cmt");
        break;
    }
}

void tr::Entity::writeComments(
        pugi::xml_node& node, WrCache& c) const
{
    writeAuthorsComment(node, c);
    writeTranslatorsComment(node, c);
}

void tr::Entity::readComments(
        const pugi::xml_node& node, const PrjInfo& info)
{
    readAuthorsComment(node);
    readTranslatorsComment(node, info);
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


void tr::VirtualGroup::writeCommentsAndChildren(
        pugi::xml_node& node, WrCache& c) const
{
    writeComments(node, c);
    for (auto& v : children) {
        v->writeToXml(node, c);
    }
}


void tr::VirtualGroup::readCommentsAndChildren(
        const pugi::xml_node& node, const PrjInfo& info)
{
    readComments(node, info);
    for (auto v : node.children()) {
        if (v.type() == pugi::node_element) {
            if (strcmp(v.name(), "text") == 0) {
                auto text = addText({}, {}, Modify::NO);
                text->readFromXml(v, info);
            } if (strcmp(v.name(), "group") == 0) {
                auto group = addGroup({}, Modify::NO);
                group->readFromXml(v, info);
            }
        }
    }
}


std::shared_ptr<tr::Group> tr::Group::clone(
        const std::shared_ptr<VirtualGroup>& parent,
        const IdLib* idlib,
        tr::Modify wantModify) const
{
    auto newId = parent->makeId<ObjType::GROUP>(idlib, this);
    auto newGroup = parent->addGroup(newId, Modify::NO);
    for (auto& v : children) {
        auto newSub = v->vclone(newGroup);
    }
    newGroup->comm = this->comm;
    if (wantModify != tr::Modify::NO) {
        newGroup->cache.mod.set(Mch::ID);
        if (!newGroup->comm.authors.empty())
            newGroup->cache.mod.set(Mch::COMMENT);
        parent->cache.mod.set(Mch::ID);
        parent->project()->modify();
    }
    return newGroup;
}


namespace {

    template <class T> requires std::is_base_of_v<tr::Entity, T>
    class CloneThing : public tr::CloneObj::Commitable
    {
    public:
        const T& that;
        std::shared_ptr<tr::VirtualGroup> parent;

        CloneThing(const T& aThat, const std::shared_ptr<tr::VirtualGroup>& aParent)
            : that(aThat), parent(aParent) {}

        std::shared_ptr<tr::UiObject> commit(
                const tr::IdLib* idlib, tr::Modify wantModify) override
            { return that.clone(parent, idlib, wantModify); }
    };

}   // anon namespace


tr::CloneObj tr::Group::startCloning(const std::shared_ptr<UiObject>& parent) const
{
    auto vg = std::dynamic_pointer_cast<VirtualGroup>(parent);
    if (!vg)
        return { CloneErr::BAD_PARENT, {} };
    return {
        CloneErr::OK,
        std::unique_ptr<CloneObj::Commitable>{ new CloneThing<tr::Group>(*this, vg) }
    };
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


void tr::Group::writeToXml(pugi::xml_node& root, WrCache& c) const
{
    auto node = root.append_child("group");
        node.append_attribute("id") = str::toC(id);
    writeCommentsAndChildren(node, c);
}


void tr::Group::readFromXml(const pugi::xml_node& node, const PrjInfo& info)
{
    id = str::toU8sv(rqAttr(node, "id").value());
    readCommentsAndChildren(node, info);
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


void tr::Text::writeToXml(pugi::xml_node& root, WrCache& c) const
{
    auto node = root.append_child("text");
        node.append_attribute("id") = str::toC(id);
    writeTextInTag(node, "orig", tr.original, c);
    writeAuthorsComment(node, c);
    switch (c.info.type) {
    case PrjType::ORIGINAL:
        break;
    case PrjType::FULL_TRANSL:
        writeTextInTagOpt(node, "known-orig", tr.knownOriginal, c);
        writeTextInTagOpt(node, "transl", tr.translation, c);
        writeTranslatorsComment(node, c);
        break;
    }
}


void tr::Text::readFromXml(const pugi::xml_node& node, const PrjInfo& info)
{
    id = str::toU8sv(rqAttr(node, "id").value());
    // Our XML is DOM-like, so we can read not in order
    //   Write: orig, au-cmt, known-orig, transl, tr-cmt
    //   Read:  au-cmt, tr-cmt, orig, known-orig, transl
    readComments(node, info);
    tr.original = readTextInTag(node, "orig");
    switch (info.type) {
    case PrjType::ORIGINAL:
        break;
    case PrjType::FULL_TRANSL:
        tr.knownOriginal = readTextInTagOpt(node, "known-orig");
        tr.translation = readTextInTagOpt(node, "transl");
        break;
    }
}


std::shared_ptr<tr::Text> tr::Text::clone(
        const std::shared_ptr<VirtualGroup>& parent,
        const IdLib* idlib,
        tr::Modify wantModify) const
{
    auto newId = parent->makeId<ObjType::TEXT>(idlib, this);
    auto newText = parent->addText(newId, {}, Modify::NO);
    newText->tr = this->tr;
    newText->comm = this->comm;
    if (wantModify != tr::Modify::NO) {
        newText->cache.mod.set(Mch::ID);
        if (!newText->tr.original.empty())
            newText->cache.mod.set(Mch::ORIG);
        if (!newText->comm.authors.empty())
            newText->cache.mod.set(Mch::COMMENT);
        parent->cache.mod.set(Mch::ID);
        parent->project()->modify();
    }
    return newText;
}


tr::CloneObj tr::Text::startCloning(const std::shared_ptr<UiObject>& parent) const
{
    auto vg = std::dynamic_pointer_cast<VirtualGroup>(parent);
    if (!vg)
        return { CloneErr::BAD_PARENT, {} };
    return {
        CloneErr::OK,
        std::unique_ptr<CloneObj::Commitable>(new CloneThing<Text>(*this, vg))
    };
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


void tr::File::writeToXml(pugi::xml_node& root, WrCache& c) const
{
    auto node = root.append_child("file");
        node.append_attribute("name") = str::toC(id);
        node.append_attribute("idless") = info.isIdless;
    writeCommentsAndChildren(node, c);
}


void tr::File::readFromXml(const pugi::xml_node& node, const PrjInfo& pinfo)
{
    id = str::toU8sv(node.attribute("name").as_string());
    info.isIdless = node.attribute("idless").as_bool(false);
    readCommentsAndChildren(node, pinfo);
}


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


bool tr::Project::unmodify(Forced forced)
{
    bool r = SimpleModifiable::unmodify(forced);
    if (r)
        recursiveUnmodify();
    return r;
}


void tr::Project::save()
{
    saveCopy(fname);
    unmodify(Forced::YES);
}


void tr::Project::save(const std::filesystem::path& aFname)
{
    saveCopy(aFname);
    fname = aFname;
    unmodify(Forced::YES);
}


void tr::Project::writeToXml(pugi::xml_node& doc) const
{
    auto root = doc.append_child("ut");
    root.append_attribute("type") = prjTypeNames[static_cast<int>(info.type)];
    WrCache c(info);
    auto nodeInfo = root.append_child("info");
        auto nodeOrig = nodeInfo.append_child("orig");
            nodeOrig.append_attribute("lang") = info.orig.lang.c_str();
    if (info.type != PrjType::ORIGINAL) {
        auto nodeTransl = nodeInfo.append_child("transl");
            nodeTransl.append_attribute("lang") = info.transl.lang.c_str();
    }
    for (auto& file : files) {
        file->writeToXml(root, c);
    }
}


void tr::Project::readFromXml(const pugi::xml_node& node)
{
    auto attrType = rqAttr(node, "type");
    info.type = parseEnumRq<PrjType>(attrType.value(), tr::prjTypeNames);
    auto nodeInfo = rqChild(node, "info");
        auto nodeOrig = rqChild(nodeInfo, "orig");
            info.orig.lang = nodeOrig.attribute("lang").as_string("en");
    if (info.type != PrjType::ORIGINAL) {
        auto nodeTransl = rqChild(nodeInfo, "transl");
            info.transl.lang = rqAttr(nodeTransl, "lang").value();
    }
    for (auto& v : node.children("file")) {
        auto file = addFile({}, Modify::NO);
        file->readFromXml(v, info);
    }
}


void tr::Project::saveCopy(const std::filesystem::path& aFname) const
{
    pugi::xml_document doc;
    auto declaration = doc.append_child(pugi::node_declaration);
        declaration.append_attribute("version") = "1.0";
        declaration.append_attribute("encoding") = "utf-8";
    writeToXml(doc);
    doc.save_file(aFname.c_str(), " ", pugi::format_indent | pugi::format_write_bom);
}


void tr::Project::load(const pugi::xml_document& doc)
{
    clear();
    auto root = rqChild(doc, "ut");
    readFromXml(root);
}


void tr::Project::load(const std::filesystem::path& aFname)
{
    pugi::xml_document doc;
    auto result = doc.load_file(aFname.c_str());
    if (!result) {
        if (result.offset)
            throw std::logic_error(std::string{result.description()}
                + ", offset=" + std::to_string(result.offset));
        throw std::logic_error(result.description());
    }
    load(doc);
    fname = aFname;
}
