#include "TrProject.h"

// C++
#include <stdexcept>

// Libs
#include "u_Strings.h"

// Pugixml
#include "pugixml.hpp"
#include "u_XmlUtils.h"

// Project
#include "TrFile.h"


using namespace std::string_view_literals;


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


///// ReadContext //////////////////////////////////////////////////////////////


std::filesystem::path tr::ReadContext::toAbsPath(const std::filesystem::path& x) const
{
    if (x.empty())
        return {};
    auto thatPath = baseDir / x;
    return std::filesystem::weakly_canonical(thatPath);
}


//// Traversable ///////////////////////////////////////////////////////////////


std::shared_ptr<tr::VirtualProject> tr::Traversable::vproject()
{
    return project();
}

std::shared_ptr<const tr::FileInfo> tr::Traversable::inheritedFileInfo() const
{
    if (auto f = file()) {
        return { f, &f->info };
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

        auto pBreak = std::find(data, end, '\n');
        if (pBreak == end) {
            // Simple write
            auto tx = node.append_child(pugi::node_pcdata);
                tx.set_value(str::toC(data));
        } else {
            // Paragraphs
            do {
                auto nextLine = node.append_child("p");
                if (pBreak != data) {
                    // Even empty pcdata kills <p /> → only when non-empty
                    auto tx = nextLine.append_child(pugi::node_pcdata);
                    tx.set_value(cache.ntsC(data, pBreak));
                }
                data = pBreak + 1;
                pBreak = std::find(data, end, '\n');
            } while (pBreak != end);
            // Last line
            auto lastLine = node.append_child("p");
            auto tx = lastLine.append_child(pugi::node_pcdata);
            tx.set_value(str::toC(data));
        }
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

    std::u8string parseTextInTagOld(pugi::xml_node tag)
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

    std::u8string parseTextInTag(pugi::xml_node tag)
    {
        if (auto para = tag.child("p")) {
            std::u8string r;
            while (true) {
                r += parseTextInTagOld(para);
                para = para.next_sibling("p");
                if (!para)
                    break;
                r += '\n';
            }
            return r;
        } else {
            return parseTextInTagOld(tag);
        }
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


void tr::Entity::writeImportersAuthorsComment(
        pugi::xml_node& node, WrCache& c) const
{
    writeTextInTagIf(node, "im-cmt", comm.importers, c);
    writeTextInTagIf(node, "au-cmt", comm.authors, c);
}

void tr::Entity::readAuthorsComment(const pugi::xml_node& node)
{
    comm.importers = readTextInTag(node, "im-cmt");
    comm.authors = readTextInTag(node, "au-cmt");
}

void tr::Entity::writeTranslatorsComment(
        pugi::xml_node& node, WrCache& c) const
{
    if (c.info.isTranslation()) {
        writeTextInTagIf(node, "tr-cmt", comm.translators, c);
    }
}

void tr::Entity::readTranslatorsComment(const pugi::xml_node& node, const PrjInfo& info)
{
    if (info.isTranslation()) {
        comm.translators = readTextInTag(node, "tr-cmt");
    }
}

void tr::Entity::writeComments(
        pugi::xml_node& node, WrCache& c) const
{
    writeImportersAuthorsComment(node, c);
    writeTranslatorsComment(node, c);
}

void tr::Entity::readComments(
        const pugi::xml_node& node, const PrjInfo& info)
{
    readAuthorsComment(node);
    readTranslatorsComment(node, info);
}


void tr::Entity::entityRemoveTranslChannel()
{
    comm.removeTranslChannel();
}


void tr::Entity::entityStealDataFrom(
        Entity& x, UiObject* myParent, const StealContext& ctx)
{
    uiStealDataFrom(x, myParent);

    // *this is EXTERNAL SOFTWARE, x is HAND-EDITED →
    // DO NOT copy importer’s comment
    switch (ctx.orig) {
    case tf::StealOrig::STEAL:
        comm.authors = x.comm.authors;
        break;
    case tf::StealOrig::KEEP:
    case tf::StealOrig::KEEP_WARN:;
    }
    comm.translators = std::move(x.comm.translators);
}


///// GroupLoader //////////////////////////////////////////////////////////////


namespace {

    ///
    /// @warning
    ///   Means for ORIGINAL only, as we create things
    ///
    class GroupLoader : public tf::Loader
    {
    public:
        GroupLoader(std::shared_ptr<tr::VirtualGroup> aRoot, tf::Existing aExisting)
            : root(std::move(aRoot)), curr(root), existing(aExisting) {}
        void goToRoot() override { curr = root; }
        bool goUp() override;
        void goToGroupRel(std::u8string_view groupId) override;
        void addText(
                std::u8string_view textId,
                std::u8string_view original,
                std::u8string_view comment) override;
    private:
        std::shared_ptr<tr::VirtualGroup> root, curr;
        tf::Existing existing;
    };

    bool GroupLoader::goUp()
    {
        if (curr == root) {
            return false;
        } else {
            curr = std::dynamic_pointer_cast<tr::VirtualGroup>(curr->parent());
            if (!curr)
                throw std::logic_error("[GroupLoader::goUp] Did not get parent");
            return true;
        }
    }

    void GroupLoader::goToGroupRel(std::u8string_view groupId)
    {
        auto q = curr->findGroup(groupId);
        if (q) {
            curr = std::move(q);
        } else {
            curr = curr->addGroup(std::u8string{groupId}, tr::Modify::YES);
        }
    }

    void GroupLoader::addText(
            std::u8string_view textId,
            std::u8string_view original,
            std::u8string_view comment)
    {
        auto text = curr->findText(textId);
        if (text) {
            // Exists
            switch (existing) {
            case tf::Existing::KEEP:
                break;  // do nothing
            case tf::Existing::OVERWRITE: {
                    text->setOriginal(original, tr::Modify::YES);
                    text->comm.importers = comment;
                }
                break;
            }
        } else {
            // Does not exist
            auto text = curr->addText(
                        std::u8string{textId}, std::u8string{original}, tr::Modify::YES);
            text->comm.importers = comment;
        }
    }

}   // anon namespace


///// VirtualGroup /////////////////////////////////////////////////////////////


std::shared_ptr<tr::UiObject> tr::VirtualGroup::child(size_t i) const
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
    r->fSelf = r;
    r->id = std::move(id);
    r->tr.original = std::move(original);
    if (wantModify != Modify::NO) {
        doModify(Mch::META);
    }
    children.push_back(r);
    cascadeDropStats();
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
        doModify(Mch::META);
    }
    children.push_back(r);
    cascadeDropStats();
    return r;
}


std::shared_ptr<tr::Project> tr::VirtualGroup::project()
{
    if (auto f = file())
        return f->project();
    return nullptr;
}


std::shared_ptr<tr::UiObject> tr::VirtualGroup::extractChild(
        size_t i, Modify wantModify)
{
    if (i >= children.size())
        return {};
    auto r = children[i];
    children.erase(children.begin() + i);
    recache();
    cascadeDropStats();
    if (wantModify != Modify::NO)
        doModify(Mch::META);
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
        const pugi::xml_node& node, const ReadContext& ctx)
{
    readComments(node, ctx.info);
    for (auto v : node.children()) {
        if (v.type() == pugi::node_element) {
            if (strcmp(v.name(), "text") == 0) {
                auto text = addText({}, {}, Modify::NO);
                text->readFromXml(v, ctx);
            } if (strcmp(v.name(), "group") == 0) {
                auto group = addGroup({}, Modify::NO);
                group->readFromXml(v, ctx);
            }
        }
    }
}


void tr::VirtualGroup::traverse(
        TraverseListener& x, tr::WalkOrder order, EnterMe enterMe)
{
    auto lk = fSelf.lock();
    if (!lk)
        throw std::logic_error("[VG.traverse] Did not lock ptr to this");

    if (enterMe != EnterMe::NO)
        x.onEnterGroup(lk);

    switch (order) {
    case tr::WalkOrder::EXACT:
        for (auto& v : children) {
            v->traverse(x, order, EnterMe::YES);
        }
        break;
    case tr::WalkOrder::ECONOMY:
        // First texts…
        for (auto& v : children) {
            if (v->translatable())
                v->traverse(x, order, EnterMe::YES);
        }
        // …then subgroups
        for (auto& v : children) {
            if (!v->translatable())
                v->traverse(x, order, EnterMe::YES);
        }
        break;
    }

    if (enterMe != EnterMe::NO)
        x.onLeaveGroup(lk);
}


void tr::VirtualGroup::doSwapChildren(size_t index1, size_t index2)
{
    if (index1 < children.size() && index2 < children.size())
        std::swap(children[index1], children[index2]);
}


std::shared_ptr<tr::Group> tr::VirtualGroup::findGroup(std::u8string_view id)
{
    for (auto& v : children) {
        if (auto vg = std::dynamic_pointer_cast<tr::Group>(v)) {
            if (vg->id == id)
                return vg;
        }
    }
    return {};
}


std::shared_ptr<tr::Text> tr::VirtualGroup::findText(std::u8string_view id)
{
    return findPText(id).obj;
}


tr::FindPText tr::VirtualGroup::findPText(std::u8string_view id)
{
    for (auto& v : children) {
        if (auto vg = std::dynamic_pointer_cast<tr::Text>(v)) {
            if (vg->id == id)
                return { &v, vg };
        }
    }
    return {};
}


void tr::VirtualGroup::loadText(
        tf::FileFormat& fmt,
        const std::filesystem::path& fname,
        tf::Existing existing)
{
    GroupLoader loader(fSelf.lock(), existing);
    fmt.doImport(loader, fname);
}


void tr::VirtualGroup::vgRemoveTranslChannel()
{
    entityRemoveTranslChannel();
    for (auto& v : children) {
        v->removeTranslChannel();
    }
}


tr::UpdateInfo tr::VirtualGroup::vgStealDataFrom(
        VirtualGroup& x, UiObject* myParent, const StealContext& ctx)
{
    entityStealDataFrom(x, myParent, ctx);
    tr::UpdateInfo r;
    // Switch state to added
    for (auto& v : children)
        v->state = ObjState::ADDED;
    // Try to steal
    for (auto& v : children) {
        switch (v->objType()) {
        case tr::ObjType::PROJECT:
        case tr::ObjType::FILE:  // They never happen inside VirtualGroup
            break;
        case tr::ObjType::GROUP: {
                auto group = std::dynamic_pointer_cast<Group>(v);
                if (!v)
                    throw std::logic_error("[vgStealDataFrom] Somehow the object is not a Group");
                if (auto xGroup = x.findGroup(v->id)) {
                    group->state = ObjState::STAYING;   // stays!
                    r += group->stealDataFrom(*xGroup, this, ctx);
                }
            } break;
        case tr::ObjType::TEXT: {
                auto text = std::dynamic_pointer_cast<Text>(v);
                if (!v)
                    throw std::logic_error("[vgStealDataFrom] Somehow the object is not a Text");
                if (auto xText = x.findPText(v->id)) {
                    text->state = ObjState::STAYING;   // stays!
                    xText.place->reset();   // Remove that text!!
                    r.changed += text->stealDataFrom(*xText.obj, this, ctx);
                }
            } break;
        }
    }

    // Add?
    for (auto& v : children) {
        if (v->state == ObjState::ADDED) {
            v->markChildrenAsAddedToday();
            r += v->addedInfo(CascadeDropCache::NO);
        }
    }
    return r;
}


void tr::VirtualGroup::vgStealReferenceFrom(VirtualGroup& x)
{
    for (auto& v : children) {
        switch (v->objType()) {
        case tr::ObjType::PROJECT:
        case tr::ObjType::FILE:  // They never happen inside VirtualGroup
            break;
        case tr::ObjType::GROUP: {
                auto group = std::dynamic_pointer_cast<Group>(v);
                if (!v)
                    throw std::logic_error("[vgStealReferenceFrom] Somehow the object is not a Group");
                if (auto xGroup = x.findGroup(v->id)) {
                    group->stealReferenceFrom(*xGroup);
                }
            } break;
        case tr::ObjType::TEXT: {
                auto text = std::dynamic_pointer_cast<Text>(v);
                if (!v)
                    throw std::logic_error("[vgStealReferenceFrom] Somehow the object is not a Text");
                if (auto xText = x.findPText(v->id)) {
                    text->stealReferenceFrom(*xText.obj);
                }
            } break;
        }
    }
}


void tr::VirtualGroup::collectSyncGroups(
        std::vector<std::shared_ptr<tr::Group>>& r)
{
    for (auto& v : children) {
        if (auto group = std::dynamic_pointer_cast<Group>(v)) {
            if (group->sync) {
                r.push_back(group);
            } else {
                group->collectSyncGroups(r);
            }
        }
    }
}


void tr::VirtualGroup::vgUpdateChildrensParents(
        const std::shared_ptr<VirtualGroup>& that)
{
    for (auto& v : children)
        v->updateParent(that);
}


void tr::VirtualGroup::markChildrenAsAddedToday()
{
    for (auto& v : children) {
        v->markChildrenAsAddedToday();
    }
}


void tr::VirtualGroup::traverseTexts(const EvText& ev)
{
    for (auto& v : children) {
        v->traverseTexts(ev);
    }
}


void tr::VirtualGroup::traverseCTexts(const EvCText& ev) const
{
    for (auto& v : children) {
        v->traverseCTexts(ev);
    }
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


namespace {

    void writeFormat(pugi::xml_node parent, tf::FileFormat* format)
    {
        if (format) {
            auto hFormat = parent.append_child("format");
            hFormat.append_attribute("name") = format->proto().techName().data();  // Tech names are const, so OK
            format->save(hFormat);
        }
    }

    std::unique_ptr<tf::FileFormat> readFormat(pugi::xml_node parent)
    {
        if (auto nodeFormat = parent.child("format")) {
            std::string_view sName = nodeFormat.attribute("name").as_string();
            if (!sName.empty()) {
                for (auto v : tf::allWorkingProtos) {
                    if (v->techName() == sName) {
                        auto format = v->make();
                        format->load(nodeFormat);
                        return format;
                    }
                }
            }
        }
        return {};
    }
}

void tr::Group::writeToXml(pugi::xml_node& root, WrCache& c) const
{
    auto node = root.append_child("group");
        node.append_attribute("id") = str::toC(id);
    if (sync) {
        auto hSync = node.append_child("sync");
        hSync.append_attribute("text-owner") =
                tf::textOwnerNames[static_cast<int>(sync.info.textOwner)];
        hSync.append_attribute("fname") =
                str::toC(c.toRelPath(sync.absPath).u8string());
        writeFormat(hSync, sync.format.get());
    }
    writeCommentsAndChildren(node, c);
}


void tr::Group::readFromXml(const pugi::xml_node& node, const ReadContext& ctx)
{
    id = str::toU8sv(rqAttr(node, "id").value());
    if (auto hSync = node.child("sync")) {
        sync.info.textOwner = parseEnumDef(
                    hSync.attribute("text-owner").as_string(),
                    tf::textOwnerNames, tf::TextOwner::ME);
        sync.absPath = ctx.toAbsPath(hSync.attribute("fname").as_string());
        sync.format = readFormat(hSync);
    }
    readCommentsAndChildren(node, ctx);
}


namespace {

    template <tr::ObjType Objt>
    std::u8string cloneIdT(
            const tr::UiObject& that, const tr::IdLib* idlib,
            const tr::UiObject* src)
    {
        if (idlib) {
            return makeIdT<Objt>(that, *idlib);
        } else if (src) {
            return std::u8string { src->idColumn() };
        } else {
            return std::u8string { that.idColumn() };
        }
    }

}


std::shared_ptr<tr::Group> tr::Group::clone(
        const std::shared_ptr<VirtualGroup>& parent,
        const IdLib* idlib,
        tr::Modify wantModify) const
{
    auto newId = cloneIdT<ObjType::GROUP>(*parent, idlib, this);
    auto newGroup = parent->addGroup(newId, Modify::NO);
    for (auto& v : children) {
        auto newSub = v->vclone(newGroup);
    }
    newGroup->comm = this->comm;
    if (wantModify != tr::Modify::NO) {
        newGroup->cache.mod.set(Mch::META);
        if (!newGroup->comm.authors.empty())
            newGroup->cache.mod.set(Mch::COMMENT);
        parent->cache.mod.set(Mch::META);
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


void tr::Group::updateParent(const std::shared_ptr<VirtualGroup>& x)
{
    fParentGroup = x;
    fFile = x->file();
    vgUpdateChildrensParents(fSelf.lock());
}


tr::UpdateInfo tr::Group::updateData()
{
    if (!sync)
        return {};
    auto prj = project();
    if (!prj)
        throw std::logic_error("[updateData] Somehow no project");

    switch (prj->info.type) {
    case tr::PrjType::ORIGINAL:
        return updateData_Original();
    case tr::PrjType::FULL_TRANSL:
        return {};
    }
    throw std::logic_error("[updateData] Strange project type");
}


tr::UpdateInfo tr::Group::updateData_Original()
{
    auto savedParent = fParentGroup.lock();

    auto tempPrj = tr::Project::make();
    auto tempFile = tempPrj->addFile(u8"tempfile", tr::Modify::NO);
    auto tempGroup = tempFile->addGroup(id, tr::Modify::NO);

    static constexpr auto NOMATTER = tf::Existing::OVERWRITE;
    tempGroup->loadText(*sync.format, sync.absPath, NOMATTER );

    std::swap(this->children, tempGroup->children);
    this->removeTranslChannel();

    StealContext ctx {
        .orig = static_cast<tf::StealOrig>(sync.info.textOwner),
    };
    auto r = this->stealDataFrom(*tempGroup, nullptr, ctx);
    auto delInfo = tempGroup->deletedInfo(CascadeDropCache::NO);
    r.deleted += delInfo;

    // Stats will always be funked up!
    updateParent(savedParent);
    stats(StatsMode::DIRECT, CascadeDropCache::YES);
    return r;
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
        if (tr.forceAttention) {
            node.append_attribute("force-attention") = true;
        }
    writeTextInTag(node, "orig", tr.original, c);
    writeImportersAuthorsComment(node, c);
    if (c.info.isTranslation()) {
        writeTextInTagOpt(node, "known-orig", tr.knownOriginal, c);
        writeTextInTagOpt(node, "transl", tr.translation, c);
        writeTranslatorsComment(node, c);
    }
}


void tr::Text::readFromXml(const pugi::xml_node& node, const ReadContext& ctx)
{
    id = str::toU8sv(rqAttr(node, "id").value());
    tr.forceAttention = node.attribute("force-attention").as_bool(false);
    // Our XML is DOM-like, so we can read not in order
    //   Write: orig, au-cmt, known-orig, transl, tr-cmt
    //   Read:  au-cmt, tr-cmt, orig, known-orig, transl
    readComments(node, ctx.info);
    tr.original = readTextInTag(node, "orig");
    switch (ctx.info.type) {
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
    auto newId = cloneIdT<ObjType::TEXT>(*parent, idlib, this);
    auto newText = parent->addText(newId, {}, Modify::NO);
    newText->tr = this->tr;
    newText->comm = this->comm;
    if (wantModify != tr::Modify::NO) {
        newText->cache.mod.set(Mch::META);
        if (!newText->tr.original.empty())
            newText->cache.mod.set(Mch::ORIG);
        if (!newText->comm.authors.empty())
            newText->cache.mod.set(Mch::COMMENT);
        parent->cache.mod.set(Mch::META);
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


tr::AttentionMode tr::Text::attentionMode() const
{
    auto prj = project();
    if (!prj)       // if so → big troubles
        return tr::AttentionMode::CALM;
    return tr.attentionMode(prj->info);
}


const tr::Stats& tr::Text::stats(StatsMode, CascadeDropCache cascade)
{
    Stats r;
    switch (attentionMode()) {
    case AttentionMode::BACKGROUND:
        r.text.nBackground = 1;
        break;
    case AttentionMode::CALM:
        r.text.nCalm = 1;
        break;
    case AttentionMode::USER_ATTENTION:
        r.text.nUserAttention = 1;
        break;
    case AttentionMode::AUTO_PROBLEM:
        r.text.nAutoProblem = 1;
        break;
    }

    if (tr.translation) {
        r.text.nTranslated = 1;
    } else {
        r.text.nUntranslated = 1;
    }

    return resetCacheIf(r, cascade);
}


void tr::Text::removeTranslChannel()
{
    entityRemoveTranslChannel();
    tr.forceAttention = false;  // attention is probably in the translation channel
    tr.knownOriginal.reset();
    tr.translation.reset();
}


tr::UpdateInfo::ByState tr::Text::stealDataFrom(
        tr::Text& x, UiObject* myParent, const StealContext& ctx)
{
    UpdateInfo::ByState r;
    entityStealDataFrom(x, myParent, ctx);
    // Translations are always kept by UT → so steal
    this->tr.translation = std::move(x.tr.translation);
    this->tr.forceAttention = x.tr.forceAttention;

    // Build stats
    const bool isOrigChanged = (this->tr.original != x.tr.original);
    tr.wasChangedToday |= (x.tr.wasChangedToday || isOrigChanged);

    auto plus1 = [this, isOrigChanged, &r] {
        if (isOrigChanged) {
            if (this->tr.translation) {
                r.nTranslated = 1;
            } else {
                r.nUntranslated = 1;
            }
        }
    };

    switch (ctx.orig) {
    case tf::StealOrig::KEEP:   // KEEP == EDITOR → change data, thus build stats
        plus1();
        break;      // do nothing, just build stats

    case tf::StealOrig::STEAL:  // STEAL == ME → originals stay in place, no stats
        this->tr.original = std::move(x.tr.original);
        this->tr.knownOriginal = std::move(x.tr.knownOriginal);
        break;

    case tf::StealOrig::KEEP_WARN: {
            plus1();

            // As x is HAND-EDITED, and *this is EXTERNAL SOFTWARE,
            // we KEEP original strings

            // First copy known original if present
            // External software has no access to knownOriginal,
            //   as this field is designed to solve contradictions between software
            //   and hand-translation
            if (x.tr.knownOriginal) {
                this->tr.knownOriginal = std::move(x.tr.knownOriginal);
            }

            if (isOrigChanged) {
                // And then make knownOriginal unless background
                if (this->tr.knownOriginal) {    // HAVE known original — maybe remove?
                    if (*this->tr.knownOriginal == this->tr.original) {
                        this->tr.knownOriginal.reset();
                    }
                } else {       // HAVE NO known original — maybe add?
                    if (this->tr.translation) {  // But add only if have translation
                        this->tr.knownOriginal = std::move(x.tr.original);
                    }
                }
            }
        } break;
    }

    return r;
}


void tr::Text::stealReferenceFrom(Text& x)
{
    this->tr.reference = std::move(x.tr.translation);
}


void tr::Text::updateParent(const std::shared_ptr<VirtualGroup>& x)
{
    fParentGroup = x;
}


void tr::Text::traverseTexts(const EvText& ev)
    { ev(*this, tr); }


void tr::Text::traverseCTexts(const EvCText& ev) const
    { ev(*this, tr); }


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
        if (!info.origPath.empty())
            node.append_attribute("orig-path") = str::toC(info.origPath.u8string());
        if (!info.translPath.empty())
            node.append_attribute("transl-path") = str::toC(info.translPath.u8string());
        writeFormat(node, info.format.get());
    writeCommentsAndChildren(node, c);
}


void tr::File::readFromXml(const pugi::xml_node& node, const ReadContext& ctx)
{
    id = str::toU8sv(node.attribute("name").as_string());
    info.isIdless = node.attribute("idless").as_bool(false);
    info.origPath = str::toU8sv(node.attribute("orig-path").as_string());
    info.translPath = str::toU8sv(node.attribute("transl-path").as_string());
    info.format = readFormat(node);
    readCommentsAndChildren(node, ctx);
}


tf::FileFormat* tr::File::exportableFormat() noexcept
{
    if (!id.empty() && info.format
            && info.format->proto().caps().have(tf::Fcap::EXPORT)) {
        return info.format.get();
    }
    return nullptr;
}


tr::HIcon tr::File::icon() const
{
    return info.format
            ? static_cast<HIcon>(&info.format->proto())
            : static_cast<HIcon>(&tf::DummyProto::INST);
}


void tr::File::removeTranslChannel()
{
    vgRemoveTranslChannel();
    info.translPath.clear();
}


tr::UpdateInfo tr::File::stealDataFrom(
        File& x, UiObject* myParent, const StealContext& ctx)
{
    switch (ctx.orig) {
    case tf::StealOrig::STEAL:
        // *this is imported, x is hand-edited, thus such things
        this->info.origPath = std::move(x.info.origPath);
        break;
    case tf::StealOrig::KEEP:
    case tf::StealOrig::KEEP_WARN:;
    }
    this->info.translPath = std::move(x.info.translPath);
    return vgStealDataFrom(x, myParent, ctx);
}


void tr::File::stealReferenceFrom(tr::File& x)
{
    vgStealReferenceFrom(x);
}


void tr::File::updateParents(const std::shared_ptr<Project>& x)
{
    fProject = x;
    vgUpdateChildrensParents(fSelf.lock());
}


///// FileWalker ///////////////////////////////////////////////////////////////

namespace {

    class FileWalker final :
            public tf::Walker, private tr::TraverseListener
    {
    public:
        FileWalker(tr::File& file,
                   tr::WalkChannel aChannel,
                   tr::WalkOrder order,
                   bool aWantPseudoLoc);
        const tf::TextInfo& nextText() override;
    private:
        struct DepthInfo {
            int commonDepth = 0, newDepth = 0;
            std::shared_ptr<const tr::Text> text = nullptr;
        };

        tr::WalkChannel channel;
        SafeVector<DepthInfo> texts;
        size_t index = 0;
        bool wantPseudoLoc = true;
        DepthInfo depthInfo;
        tf::TextInfo textInfo;
        std::u8string pseudoLoced;

        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x) override;
        void onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>& x) override;
        void onText(const std::shared_ptr<tr::Text>& x) override;
        std::u8string_view pseudoLoc(std::u8string_view s);
    };

    void FileWalker::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&)
    {
        ++depthInfo.newDepth;
    }

    void FileWalker::onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&)
    {
        if ((--depthInfo.newDepth) < depthInfo.commonDepth) {
            depthInfo.commonDepth = depthInfo.newDepth;
        }
    }

    void FileWalker::onText(const std::shared_ptr<tr::Text>& x)
    {
        auto& v = texts.emplace_back(depthInfo);
        v.text = x;
        depthInfo.commonDepth = depthInfo.newDepth;
    }

    FileWalker::FileWalker(
            tr::File& file, tr::WalkChannel aChannel, tr::WalkOrder order,
            bool aWantPseudoLoc)
        : channel(aChannel), wantPseudoLoc(aWantPseudoLoc)
    {
        file.traverse(*this, order, tr::EnterMe::NO);
    }

    std::u8string_view FileWalker::pseudoLoc(std::u8string_view s)
    {
        static constexpr std::u8string_view EPAULET1 = u8"‹§ƻ-";
        static constexpr std::u8string_view EPAULET2 = u8"-αԶ›";
        pseudoLoced.clear();
        pseudoLoced.reserve(s.length() + EPAULET1.length() + EPAULET2.length());
        pseudoLoced.append(EPAULET1);
        pseudoLoced.append(s);
        pseudoLoced.append(EPAULET2);
        return pseudoLoced;
    }

    const tf::TextInfo& FileWalker::nextText()
    {
        // Shift depth
        textInfo.prevDepth = textInfo.actualDepth();        // 1. prevDepth

        if (index >= texts.size()) {
            // End?
            textInfo.commonDepth = 0;
            textInfo.ids.clear();
        } else {
            // OK
            auto& di = texts[index++];
            textInfo.commonDepth = di.commonDepth;          // 2. commonDepth
            auto depth = di.newDepth;

            // Set IDs, assign text ID
            textInfo.ids.resize(depth + 1);                 // 3. ids
            textInfo.ids.back() = di.text->id;

            // Assign rest IDs from depth to commonDepth
            auto pText = di.text->parent();
            while (depth > di.commonDepth) {
                if (!pText)
                    throw std::logic_error("[FileWalker.nextText] File was destroyed somehow");
                textInfo.ids[--depth] = pText->idColumn();
                pText = pText->parent();
            }

            // Assign fixed text
            textInfo.original = di.text->tr.original;       // 4. original
            textInfo.translation = di.text->tr.translationSv(); // 5. translation
            // Assign text of channel
            switch (channel) {                              // 6. text
            case tr::WalkChannel::ORIGINAL:
                textInfo.text = textInfo.original;
                textInfo.isFallbackLocale = false;
                break;
            case tr::WalkChannel::TRANSLATION:
                if (di.text->tr.translation) {
                    textInfo.text = textInfo.translation;
                    textInfo.isFallbackLocale = false;
                } else {
                    textInfo.text = wantPseudoLoc
                            ? pseudoLoc(textInfo.original)
                            : textInfo.original;
                    textInfo.isFallbackLocale = true;
                }
                break;
            }
        }
        return textInfo;
    }

}   // anon namespace

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


std::shared_ptr<tr::File> tr::Project::addFile(
        std::u8string_view name, Modify wantModify)
{
    auto index = nChildren();
    auto r = std::make_shared<File>(fSelf.lock(), index, PassKey{});
    r->fSelf = r;
    r->id = name;
    if (wantModify != Modify::NO) {
        doModify(Mch::META);
    }
    files.push_back(r);
    cascadeDropStats();
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


std::shared_ptr<tr::UiObject> tr::Project::extractChild(
        size_t i, Modify wantModify)
{
    if (i >= files.size())
        return {};
    auto r = files[i];
    files.erase(files.begin() + i);
    recache();
    cascadeDropStats();
    if (wantModify != Modify::NO)
        doModify(Mch::META);
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


void tr::Project::writeToXml(
        pugi::xml_node& doc,
        const std::filesystem::path& basePath) const
{
    auto root = doc.append_child("ut");
    root.append_attribute("type") = prjTypeNames[info.type];
    WrCache c(info, basePath);
    auto nodeInfo = root.append_child("info");
        auto nodeOrig = nodeInfo.append_child("orig");
            nodeOrig.append_attribute("lang") = info.orig.lang.c_str();
            if (info.hasOriginalPath()) {
                if (!info.orig.absPath.empty()) {
                    auto relPath = c.toRelPath(info.orig.absPath);
                    nodeOrig.append_attribute("fname") = str::toC(relPath.u8string());
                }
            }
    if (info.canHaveReference() && !info.ref.absPath.empty()) {
        auto nodeRef = nodeInfo.append_child("ref");
            auto relPath = c.toRelPath(info.ref.absPath);
            nodeRef.append_attribute("fname") = str::toC(relPath.u8string());
    }
    if (info.isTranslation()) {
        auto nodeTransl = nodeInfo.append_child("transl");
            nodeTransl.append_attribute("lang") = info.transl.lang.c_str();
            if (info.isFullTranslation()) {
                bool hasPseudoloc = info.transl.pseudoloc.isOn();
                nodeTransl.append_attribute("pseudoloc") = hasPseudoloc;
            }
    }
    for (auto& file : files) {
        file->writeToXml(root, c);
    }
}


void tr::Project::readFromXml(
        const pugi::xml_node& node,
        const std::filesystem::path& basePath)
{
    ReadContext ctx {
        .info = info,
        .baseDir = basePath,
    };
    auto attrType = rqAttr(node, "type");
    info.type = parseEnumRq<PrjType>(attrType.value(), tr::prjTypeNames.cArray());
    auto nodeInfo = rqChild(node, "info");
        auto nodeOrig = rqChild(nodeInfo, "orig");
            info.orig.lang = nodeOrig.attribute("lang").as_string("en");
            if (info.hasOriginalPath()) {
                info.orig.absPath = ctx.toAbsPath(nodeOrig.attribute("fname").as_string());
            }
    if (info.canHaveReference()) {
        auto nodeRef = nodeInfo.child("ref");
        info.ref.absPath = ctx.toAbsPath(nodeRef.attribute("fname").as_string());
    }
    if (info.isTranslation()) {
        auto nodeTransl = rqChild(nodeInfo, "transl");
            info.transl.lang = rqAttr(nodeTransl, "lang").value();
            if (nodeTransl.attribute("pseudoloc").as_bool(false)) {
                info.transl.pseudoloc = tr::PrjInfo::Transl::Pseudoloc::DFLT;
            } else {
                info.transl.pseudoloc = tr::PrjInfo::Transl::Pseudoloc::OFF;
            }
    }
    for (auto& v : node.children("file")) {
        auto file = addFile({}, Modify::NO);
        file->readFromXml(v, ctx);
    }
}


void tr::Project::saveCopy(const std::filesystem::path& aFname) const
{
    pugi::xml_document doc;
    auto declaration = doc.append_child(pugi::node_declaration);
        declaration.append_attribute("version") = "1.0";
        declaration.append_attribute("encoding") = "utf-8";
    writeToXml(doc, aFname.parent_path());
    doc.save_file(aFname.c_str(), " ", pugi::format_indent | pugi::format_write_bom);
}


void tr::Project::load(
        const pugi::xml_document& doc,
        const std::filesystem::path& basePath)
{
    clear();
    auto root = rqChild(doc, "ut");
    readFromXml(root, basePath);
}


void tr::Project::load(const std::filesystem::path& aFname)
{
    pugi::xml_document doc;
    auto result = doc.load_file(aFname.c_str(),
                pugi::parse_default | pugi::parse_ws_pcdata);
    xmlThrowIf(result);
    load(doc, aFname.parent_path());
    fname = aFname;
}


size_t tr::Project::nOrigExportableFiles() const
{
    size_t r = 0;
    for (auto& f : files) {
        switch (f->mode()) {
        case FileMode::HOSTED:
            if (f->info.format
                    && f->info.format->proto().caps().have(tf::Fcap::EXPORT)) {
                ++r;
            }
            break;
        case FileMode::EXTERNAL:
            // External files are not exported in original mode
            break;
        }
    }
    return r;
}


void tr::Project::traverse(TraverseListener& x, tr::WalkOrder order, tr::EnterMe)
{
    for (auto& v : files)
        v->traverse(x, order, EnterMe::YES);
}


void tr::Project::doSwapChildren(size_t index1, size_t index2)
{
    if (index1 < files.size() && index2 < files.size())
        std::swap(files[index1], files[index2]);
}


tr::WalkChannel tr::Project::walkChannel() const
{
    switch (info.type) {
    case PrjType::ORIGINAL:
        return WalkChannel::ORIGINAL;
    case PrjType::FULL_TRANSL:
        return WalkChannel::TRANSLATION;
    }
    throw std::logic_error("[Project.walkChannel] Strange project type");
}


void tr::Project::doBuild(const std::filesystem::path& destDir)
{
    auto fullName = fname;
    auto saveDir = fullName.parent_path();
    auto stem = fullName.stem();
    auto subdir = L"build-" + stem.wstring();
    auto defaultExportDir = saveDir / subdir;

    for (auto& file : files) {
        if (auto format = file->exportableFormat()) {
            std::filesystem::path fnAsked = file->id;
            // Get export filename:
            // • bare filename → save/build-xxx/filename.ext
            // • filename w/path component → save/path/finename.ext
            // • absolute path → c:/path/filename.ext
            auto fnExisting =
                    fnAsked.is_absolute()
                        ? fnAsked                       // 3) abs path
                        : (fnAsked.has_parent_path()    // …rel path…
                            ? saveDir / fnAsked                 // 2) filename+path
                            : defaultExportDir / fnAsked);      // 1) bare filename
            std::filesystem::path fnExported, dirExported;
            /// @todo [urgent, #22] file ID / file name
            if (destDir.empty()) {
                if (format->proto().caps().have(tf::Fcap::NEEDS_FILE)) {
                    // Needs file → do not touch it, create at defaultExportDir
                    dirExported = defaultExportDir;
                    fnExported = dirExported / fnExisting.filename();
                } else {
                    // Does not need → work as usually
                    fnExported = fnExisting;
                    dirExported = fnExported.parent_path();
                }
            } else {
                dirExported = destDir;
                fnExported = dirExported / fnExisting.filename();
            }
            try {
                std::filesystem::create_directories(dirExported);
                FileWalker walker(*file, walkChannel(), format->walkOrder(),
                                  info.wantPseudoLoc());
                format->doExport(walker, fnExisting, fnExported);
            } catch (...) {
                /// @todo [bug, #45] We hide errors here
            }
        }
    }
}


std::u8string tr::Project::shownFname(std::u8string_view fallback)
{
    auto r = fname.filename().u8string();
    if (r.empty()) {
        return std::u8string { fallback } ;
    } else {
        return r;
    }
}


std::shared_ptr<tr::File> tr::Project::findFile(std::u8string_view aId)
{
    for (auto& v : files) {
        if (v && v->id == aId)
            return v;
    }
    return nullptr;
}


void tr::Project::updateReference()
{
    removeReferenceChannel();
    if (!info.hasReference())
        return;

    auto tempPrj = tr::Project::make();
    tempPrj->load(this->info.ref.absPath);
    stealReferenceFrom(*tempPrj);
}


tr::UpdateInfo tr::Project::updateData(TrashMode mode)
{
    switch (info.type) {
    case tr::PrjType::ORIGINAL:
        return { .isOriginal = true };
    case tr::PrjType::FULL_TRANSL:
        return updateData_FullTransl(mode);
    }
    throw std::logic_error("[updateData] Strange project type");
}


void tr::Project::removeTranslChannel()
{
    for (auto& v : files)
        v->removeTranslChannel();
}


tr::UpdateInfo tr::Project::stealDataFrom(tr::Project& x, const StealContext& ctx)
{
    tr::UpdateInfo r;
    // Switch state to added
    for (auto& v : files)
        v->state = ObjState::ADDED;
    // Try to steal
    for (auto& v : files) {
        if (auto xFile = x.findFile(v->id)) {
            v->state = ObjState::STAYING;   // stays!
            r += v->stealDataFrom(*xFile, this, ctx);
        }
    }
    // Add?
    for (auto& v : files) {
        if (v->state == ObjState::ADDED)
            r += v->addedInfo(CascadeDropCache::NO);
    }
    // Stats
    auto delInfo = x.deletedInfo(CascadeDropCache::NO);
    r.deleted += delInfo;
    return r;
}


void tr::Project::stealReferenceFrom(tr::Project& x)
{
    // Try to steal
    for (auto& v : files) {
        if (auto xFile = x.findFile(v->id)) {
            v->stealReferenceFrom(*xFile);
        }
    }
}


tr::UpdateInfo tr::Project::updateData_FullTransl(TrashMode mode)
{
    auto tempPrj = tr::Project::make();
    tempPrj->load(this->info.orig.absPath);
    // Info and fname are left intact
    std::swap(this->files, tempPrj->files);
    // Copy original language
    if (!tempPrj->info.orig.lang.empty())
        this->info.orig.lang = tempPrj->info.orig.lang;
    this->removeTranslChannel();    // Do not forget, we swapped!
    StealContext ctx {
        .orig = tf::StealOrig::KEEP_WARN,
    };
    auto r = this->stealDataFrom(*tempPrj, ctx);
    // Stats will always be funked up!
    updateParents();
    stats(StatsMode::DIRECT, CascadeDropCache::NO);
    return r;
}


void tr::Project::updateParents()
{
    auto that = fSelf.lock();
    for (auto& v : files) {
        v->updateParents(that);
    }
}

std::vector<std::shared_ptr<tr::Group>> tr::Project::syncGroups()
{
    std::vector<std::shared_ptr<tr::Group>> r;
    for (auto& v : files) {
        v->collectSyncGroups(r);
    }
    return r;
}


void tr::Project::markChildrenAsAddedToday()
{
    for (auto& v : files) {
        v->markChildrenAsAddedToday();
    }
}


void tr::Project::traverseTexts(const EvText& ev)
{
    for (auto& v : files) {
        v->traverseTexts(ev);
    }
}


void tr::Project::traverseCTexts(const EvCText& ev) const
{
    for (auto& v : files) {
        v->traverseCTexts(ev);
    }
}
