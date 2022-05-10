#pragma once

// STL
#include <memory>
#include <atomic>
#include <optional>
#include <filesystem>

// Translator
#include "TrDefines.h"
#include "Modifiable.h"

// Libs
#include "u_Vector.h"

namespace pugi {
    class xml_document;
    class xml_node;
}

namespace detail {
    enum class TriBool : unsigned char { NO, UNK, YES };
}

class TriBool
{
public:
    static constexpr detail::TriBool NO = detail::TriBool::NO;
    static constexpr detail::TriBool UNK = detail::TriBool::UNK;
    static constexpr detail::TriBool YES = detail::TriBool::YES;
    friend constexpr bool operator == (TriBool, detail::TriBool) noexcept;
    friend constexpr bool operator == (detail::TriBool, TriBool) noexcept;
    constexpr TriBool() noexcept = default;
    constexpr TriBool(detail::TriBool x) noexcept : d(x) {}
    constexpr TriBool(bool x) noexcept : d(x ? YES : NO) {}
    constexpr bool isYes() const noexcept { return (d == YES); }
    constexpr bool isNo() const noexcept { return (d == NO); }
    constexpr bool isUnk() const noexcept { return (d == UNK); }
    constexpr bool isSet() const noexcept { return (d != UNK); }
private:
    detail::TriBool d = detail::TriBool::UNK;
};

constexpr bool operator == (TriBool x, detail::TriBool y) noexcept { return (x.d == y); }
constexpr bool operator == (detail::TriBool x, TriBool y) noexcept { return (x == y.d); };


namespace tr {

    /// Modification channel
    enum class Mch {
        META    = 1,
        ID      = 2,
        ORIG    = 4,
        TRANSL  = 8,
        COMMENT = 10,   ///< as we do not show comments in table, let it be…
        META_ID = META | ID,
    };

    class Mod {
    public:
        void clear() { v = 0; }
        void set(Mch ch) { v |= static_cast<int>(ch); }
        bool has(Mch ch) const { return v & static_cast<int>(ch); }
    private:
        unsigned v = 0;
    };

    template <class T>
    struct Pair {
        std::shared_ptr<T> first, second;

        unsigned size() const;
        Pair() = default;
        Pair(std::shared_ptr<T> only) : first(std::move(only)) {}  ///< implicit OK!
        Pair(std::shared_ptr<T> x, std::shared_ptr<T> y)
            : first(std::move(x)), second(std::move(y)) {}
        /// @return [+] its size is 2  [-] fewer
        bool is2() const { return first && second; }
    };

    class VirtualGroup;
    class Group;
    class File;
    class Project;
    class Text;
    class UiObject;

    enum class ObjType { PROJECT, FILE, GROUP, TEXT };

    enum class ObjState { STAYING, ADDED, DELETED };

    class Entity;

    template <class T>
    class Self
    {
    public:
        Self() = default;
        Self(const Self&) {}
        Self(Self&&) noexcept {}
        Self& operator = (const Self&) { return *this; }
        Self& operator = (Self&&) noexcept { return *this; }
        std::weak_ptr<T> fSelf;
    };

    class CanaryObject
    {
    public:
        /// Ctors and op= do nothing: canary depends on pointer
        CanaryObject();
        CanaryObject(const CanaryObject&) : CanaryObject() {}
        CanaryObject& operator=(const CanaryObject&) { return *this; }
        /// No virtual dtor here!
        ~CanaryObject();
        /// Checks whether canary is OK
        void checkCanary() const;
    protected:
        volatile std::atomic<uint32_t> canary  = 0;
        uint32_t goodCanary() const;
    };

    struct Comments {
        std::u8string authors, translators;

        void removeTranslChannel() { translators.clear(); }
    };

    enum class AttentionMode {
        BACKGROUND,     ///< patch translation only — untouched (grey)
        CALM,           ///< normal (green)
        ATTENTION       ///< attention (red)
    };

    struct Translatable {
        std::u8string original;     ///< Current original string
        std::optional<std::u8string>
                    knownOriginal,  ///< Known original string we translated (never == original!)
                    translation;    ///< Translation for known original (if present) or original
        bool forceAttention = false;
        std::u8string_view translationSv() const
            { return translation ? *translation : std::u8string_view(); }
        AttentionMode attentionMode(const tr::PrjInfo& prjInfo) const;
    };

    enum class Modify { NO, YES };

    /// Simple cache to speed up writing
    struct WrCache {
        const PrjInfo& info;
        std::u8string u8;

        WrCache(const PrjInfo& aInfo) : info(aInfo) {}

        /// Ensures UTF-8 string length + 8 additional bytes
        void ensureU8(size_t length);
        /// Turns beg..end to null-terminated string
        const char8_t* nts(const char8_t* beg, const char8_t* end);
        const char* ntsC(const char8_t* beg, const char8_t* end)
            { return reinterpret_cast<const char*>(nts(beg, end)); }
    };

    struct IdLib {
        std::u8string_view filePrefix;
        std::u8string_view fileSuffix;
        std::u8string_view groupPrefix;
        std::u8string_view textPrefix;
    };

    enum class CloneErr {
        OK,
        UNCLONEABLE,
        BAD_PARENT,
        BAD_OBJECT      ///< Unused in lib, UI only
    };

    struct CloneObj {
        class Commitable {
        public:
            virtual std::shared_ptr<UiObject> commit(
                    const IdLib* idlib, Modify wantModify) = 0;
            virtual ~Commitable() = default;
        };

        CloneErr err;
        std::unique_ptr<Commitable> action;
        explicit operator bool() const { return static_cast<bool>(action); }

        std::shared_ptr<UiObject> commit(const IdLib* idlib, Modify wantModify)
        {
            if (action)
                return action->commit(idlib, wantModify);
            return {};
        }
    };

    struct UiFileInfo {
        tr::FileInfo* info;
        tf::ProtoFilter filter;
    };

    class TraverseListener {    // interface
    public:
        virtual void onText(const std::shared_ptr<Text>&) = 0;
        virtual void onEnterGroup(const std::shared_ptr<VirtualGroup>&) {}
        virtual void onLeaveGroup(const std::shared_ptr<VirtualGroup>&) {}
    };

    enum class EnterMe { NO, YES };

    enum class CascadeDropCache { NO, YES };

    enum class StatsMode {
        CACHED,         ///< Use cache if present
        SEMICACHED,     ///< Do not use cache for my object
        DIRECT          ///< Do not use cache at all
    };

    struct Stats {
        size_t nGroups = 0;  ///< # of groups NOT INCLUDING me
        struct Text {
            size_t nBackground = 0, nCalm = 0, nAttention = 0;
            size_t nTotal() const noexcept { return nBackground + nCalm + nAttention; }
            bool operator == (const Text& x) const = default;
        } text;
        bool isGroup = false;

        void clear() { *this = Stats(); }
        Stats& operator += (const Stats& x);
        bool operator == (const Stats& x) const = default;
    };

    struct UpdateInfo {
        size_t nAdded = 0;
        struct ByAttent {
            size_t nCalmToAtention = 0;
            size_t nAlreadyAttention = 0;
            size_t nBackground = 0;

            ByAttent& operator += (const ByAttent& x);
            bool operator == (const ByAttent& x) const = default;
        } deleted, changed;

        UpdateInfo& operator += (const UpdateInfo& x);
        bool operator == (const UpdateInfo& x) const = default;
        bool hasSmth() const { return (*this != ZERO); }

        static const UpdateInfo ZERO;
    };

    class UiObject : public CanaryObject
    {
    public:
        struct Cache {
            int index = -1;             ///< index in tree
            Mod mod;
            TriBool isExpanded;
            std::optional<Stats> stats;
        } cache;
        // Just here we use virtual dtor!
        virtual ~UiObject() = default;

        virtual ObjType objType() const noexcept = 0;
        virtual std::shared_ptr<UiObject> parent() const = 0;
        virtual size_t nChildren() const noexcept = 0;
        virtual std::shared_ptr<Entity> child(size_t i) const = 0;
        virtual std::u8string_view idColumn() const = 0;
        virtual std::u8string_view origColumn() const { return {}; }
        /// @return [+] was changed
        virtual bool setId(std::u8string_view, tr::Modify) { return false; }
        /// Deletes i’th child
        /// @return extracted child
        /// @warning  Because of recache, complexity is O(n)
        virtual std::shared_ptr<Entity> extractChild(size_t, Modify) { return {}; }
        virtual std::shared_ptr<File> file() = 0;        

        /// @return  ptr to comments, or null
        virtual Comments* comments() { return nullptr; }
        /// @return  ptr to original/translation, or null
        virtual Translatable* translatable() { return nullptr; }
        /// @return  ptr to file info, or null
        virtual FileInfo* ownFileInfo() { return nullptr; }
        /// @return  ptr to own file foprmat, or null
        virtual CloningUptr<tf::FileFormat>* ownFileFormat() { return nullptr; }
        /// Goes together with ownFileFormat. Checks which formats are allowed
        virtual const tf::ProtoFilter* allowedFormats() const { return nullptr; }
        /// @return  ptr to project
        virtual std::shared_ptr<Project> project() = 0;
        /// @return  one or two parent groups for “Add group” / “Add string”
        virtual Pair<VirtualGroup> additionParents() = 0;
        /// @param [in]  idlib   [+] ID library; [0] copy ID
        ///              should exist until commit!
        virtual CloneObj startCloning(
                [[maybe_unused]] const std::shared_ptr<UiObject>& parent) const
            { return { CloneErr::UNCLONEABLE, {} }; }
        virtual void traverse(
                TraverseListener& x, tr::WalkOrder order, EnterMe enterMe) = 0;
        virtual std::shared_ptr<VirtualGroup> nearestGroup() = 0;
        virtual HIcon icon() const { return nullptr; }

        /// Makes nChildren == 0
        /// @warning For project: clear() makes a brand new project
        ///                       clearChildren() just removes all files
        virtual void clearChildren() = 0;

        /// Gets statistics, can use cache
        /// @warning Should work with nulls instead of some objects!
        virtual const Stats& stats(StatsMode mode, CascadeDropCache cascade);
        UpdateInfo addedInfo(CascadeDropCache cascade);
        UpdateInfo::ByAttent deletedInfo(CascadeDropCache cascade);

        /// @return self as shared_ptr
        virtual std::shared_ptr<UiObject> selfUi() = 0;

        /// Removes everything related to translation, leaving only original
        virtual void removeTranslChannel() = 0;

        void recache();
        void recursiveRecache();
        void recursiveUnmodify();

        // Utils
        /// @return [+] was actually changed
        bool setOriginal(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setAuthorsComment(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setTranslation(std::optional<std::u8string_view> x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setTranslatorsComment(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setIdless(bool x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setOrigPath(const std::filesystem::path& x, tr::Modify wantModify);
        /// @return some ID
        std::u8string makeId(
                std::u8string_view prefix,
                std::u8string_view suffix) const;
        std::u8string makeTextId(const IdLib& idlib) const;
        template <ObjType Objt>
        inline std::u8string makeId(const IdLib& idlib) const
        {
            if constexpr (Objt == ObjType::PROJECT) {
                return u8"project";
            } else if constexpr (Objt == ObjType::FILE) {
                return makeId(idlib.filePrefix, idlib.fileSuffix);
            } else if constexpr (Objt == ObjType::GROUP) {
                return makeId(idlib.groupPrefix, {});
            } else if constexpr (Objt == ObjType::TEXT) {
                return makeTextId(idlib);
            } else {
                throw std::logic_error("[UiObject.makeId] Strange objType");
            }
        }
        template <ObjType Objt>
        std::u8string makeId(const IdLib* idlib, const UiObject* src) const;
        /// @return  fileInfo, either own or inherited from file
        const FileInfo* inheritedFileInfo() const;

        /// @return  [+] s_p to this  [0] nothing happened
        std::shared_ptr<Entity> extract(Modify wantModify);
        /// Adds statistics about a single object (not children)
        void doModify(Mch ch);
        bool canMoveUp(const UiObject* aChild) const;
        bool canMoveDown(const UiObject* aChild) const;
        bool moveUp(UiObject* aChild);
        bool moveDown(UiObject* aChild);
        void swapChildren(size_t index1, size_t index2);        

        // Const verions
        const FileInfo* ownFileInfo() const
            { return const_cast<UiObject*>(this)->ownFileInfo(); }
        std::shared_ptr<const File> file() const
            { return const_cast<UiObject*>(this)->file(); }
        std::shared_ptr<const Project> project() const
            { return const_cast<UiObject*>(this)->project(); }
    protected:
        // passkey idiom
        struct PassKey {};

        /// Exchanges child[i] and child[i+1]
        virtual void doSwapChildren(size_t index1, size_t index2) = 0;
        const Stats& resetCacheIf(const Stats& r, CascadeDropCache cascade);
        void cascadeDropStats();
    };

    class Entity : public UiObject
    {
    public:
        std::u8string id;               ///< Identifier of group or string
        Comments comm;
        ObjState state = ObjState::STAYING;

        std::u8string_view idColumn() const override { return id; }
        Comments* comments() override { return &comm; }
        bool setId(std::u8string_view x, tr::Modify wantModify) override;

        /// Writes object to XML
        /// @param [in,out] root  tag ABOVE, should create a new one for entity
        /// @param [in] c         some info that speeds up saving
        virtual void writeToXml(pugi::xml_node& root, WrCache& c) const = 0;
        /// Reads object from XML
        /// @param [in] node   tag of THIS OBJECT
        /// @param [in] info   project info for speed
        virtual void readFromXml(const pugi::xml_node& node, const PrjInfo& info) = 0;
        virtual std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>& parent) const = 0;        
    protected:
        // write comments
        void writeAuthorsComment(pugi::xml_node& node, WrCache& c) const;
        void writeTranslatorsComment(pugi::xml_node& node, WrCache& c) const;
        void writeComments(pugi::xml_node& node, WrCache&) const;
        // read comments
        void readAuthorsComment(const pugi::xml_node& node);
        void readTranslatorsComment(const pugi::xml_node& node, const PrjInfo& info);
        void readComments(const pugi::xml_node& node, const PrjInfo& info);
        void entityRemoveTranslChannel();
        void entityStealDataFrom(Entity& x);
    };

    struct FindPText {
        std::shared_ptr<Entity>* place = nullptr;
        std::shared_ptr<Text> obj {};
        explicit operator bool () const { return place; }
    };

    class VirtualGroup : public Entity, protected Self<VirtualGroup>
    {
    private:
        using Super = Entity;
    public:
        SafeVector<std::shared_ptr<Entity>> children;

        size_t nChildren() const noexcept override { return children.size(); };
        std::shared_ptr<Entity> child(size_t i) const override;
        std::shared_ptr<Entity> extractChild(size_t i, Modify wantModify) override;
        void traverse(
                TraverseListener& x, tr::WalkOrder order, EnterMe enterMe) override;

        using Super::Super;

        std::shared_ptr<Text> addText(
                std::u8string id, std::u8string original,
                Modify wantModify);
        std::shared_ptr<Group> addGroup(
                std::u8string id, Modify wantModify);
        std::shared_ptr<Project> project() override;
        std::shared_ptr<VirtualGroup> nearestGroup() override { return fSelf.lock(); }
        void loadText(
                tf::FileFormat& fmt,
                const std::filesystem::path& fname,
                tf::Existing existing);
        std::shared_ptr<Group> findGroup(std::u8string_view id);
        std::shared_ptr<Text> findText(std::u8string_view id);
        FindPText findPText(std::u8string_view id);
        void clearChildren() override { children.clear(); cascadeDropStats(); }
        std::shared_ptr<UiObject> selfUi() override { return fSelf.lock(); }
    protected:
        friend class Project;
        void doSwapChildren(size_t index1, size_t index2) override;

        void writeCommentsAndChildren(pugi::xml_node&, WrCache&) const;
        void readCommentsAndChildren(const pugi::xml_node& node, const PrjInfo& info);
        void vgRemoveTranslChannel();
        UpdateInfo vgStealDataFrom(VirtualGroup& x);
    };

    class Text final : public Entity, protected Self<Text>
    {
    public:
        Translatable tr;

        Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const PassKey&);

        ObjType objType() const noexcept override { return ObjType::TEXT; }
        size_t nChildren() const noexcept override { return 0; };
        std::shared_ptr<Entity> child(size_t) const override { return {}; }
        std::u8string_view origColumn() const override { return tr.original; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        Pair<VirtualGroup> additionParents() override { return fParentGroup.lock(); }
        Translatable* translatable() override { return &tr; }
        std::shared_ptr<File> file() override;
        std::shared_ptr<Project> project() override;
            using Entity::project;
        void writeToXml(pugi::xml_node&, WrCache&) const override;
        void readFromXml(const pugi::xml_node& node, const PrjInfo& info) override;
        bool isCloneable() const noexcept { return true; }
        void traverse(TraverseListener& x, tr::WalkOrder, EnterMe) override
            { x.onText(fSelf.lock()); }
        std::shared_ptr<VirtualGroup> nearestGroup() override { return fParentGroup.lock(); }
        std::shared_ptr<Text> clone(
                const std::shared_ptr<VirtualGroup>& parent,
                const IdLib* idlib,
                tr::Modify wantModify) const;
        CloneObj startCloning(
                const std::shared_ptr<UiObject>& parent) const override;
        AttentionMode attentionMode() const;
        void clearChildren() override {}
        const Stats& stats(StatsMode mode, CascadeDropCache cascade) override;
        std::shared_ptr<UiObject> selfUi() override { return fSelf.lock(); }
        void removeTranslChannel() override;
        ///  @return  CHANGED data
        UpdateInfo::ByAttent stealDataFrom(Text& x);
    protected:
        std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>& parent) const override
            { return clone(parent, nullptr, Modify::NO); }
        void doSwapChildren(size_t, size_t) override {}
    private:
        std::weak_ptr<VirtualGroup> fParentGroup;
        friend class VirtualGroup;
    };

    class Group final : public VirtualGroup
    {
    private:
        using Super = VirtualGroup;
    public:
        std::unique_ptr<tf::FileFormat> linkedFile;

        Group(const std::shared_ptr<VirtualGroup>& aParent,
              size_t aIndex, const PassKey&);

        ObjType objType() const noexcept override { return ObjType::GROUP; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        std::shared_ptr<File> file() override { return fFile.lock(); }
        Pair<VirtualGroup> additionParents() override
                { return { fParentGroup.lock(), fSelf.lock() }; }
        void writeToXml(pugi::xml_node&, WrCache&) const override;
        void readFromXml(const pugi::xml_node& node, const PrjInfo& info) override;
        std::shared_ptr<Group> clone(
                const std::shared_ptr<VirtualGroup>& parent,
                const IdLib* idlib,
                tr::Modify wantModify) const;
        CloneObj startCloning(
                const std::shared_ptr<UiObject>& parent) const override;
        std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>& parent) const override
            { return clone(parent, nullptr, Modify::NO); }
        void removeTranslChannel() override { vgRemoveTranslChannel(); }
        UpdateInfo stealDataFrom(Group& x) { return vgStealDataFrom(x); }
    private:
        friend class tr::File;
        std::weak_ptr<File> fFile;
        std::weak_ptr<VirtualGroup> fParentGroup;
    };

    enum class FileMode { HOSTED, EXTERNAL };

    class File final : public VirtualGroup
    {
    private:
        using Super = VirtualGroup;
    public:
        FileInfo info;

        std::shared_ptr<Project> project() override { return fProject.lock(); }

        ObjType objType() const noexcept override { return ObjType::FILE; }
        std::shared_ptr<UiObject> parent() const override;
        std::shared_ptr<File> file() override
            { return std::dynamic_pointer_cast<File>(fSelf.lock()); }
        Pair<VirtualGroup> additionParents() override { return fSelf.lock(); }

        File(std::weak_ptr<Project> aProject, size_t aIndex, const PassKey&);
        void writeToXml(pugi::xml_node&, WrCache&) const override;
        void readFromXml(const pugi::xml_node& node, const PrjInfo& info) override;
        using Super::ownFileInfo;
        virtual FileInfo* ownFileInfo() override { return &info; }
        std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>&) const override
            { throw std::logic_error("Cannot clone files"); }
        CloningUptr<tf::FileFormat>* ownFileFormat() override { return &info.format; }
        const tf::ProtoFilter* allowedFormats() const override
            { return &tf::ProtoFilter::ALL_EXPORTING_AND_NULL; }
        HIcon icon() const override;
        void removeTranslChannel() override;

        constexpr FileMode mode() const noexcept { return FileMode::HOSTED; }
        tf::FileFormat* exportableFormat() noexcept;        
        UpdateInfo stealDataFrom(File& x);
    protected:
        std::weak_ptr<Project> fProject;
    };

    enum class WalkChannel { ORIGINAL, TRANSLATION };

    class Project final :
            public UiObject,
            public SimpleModifiable,
            private Self<Project>
    {
    public:
        PrjInfo info;
        std::filesystem::path fname;
        SafeVector<std::shared_ptr<File>> files;

        /// @brief addTestOriginal
        ///   Adds a few files and strings that will serve as test original
        ///   (project will be original only, w/o translation)
        void addTestOriginal();

        /// Ctors are private, op= is the same
        Project& operator = (const Project&) = default;
        Project& operator = (Project&&) noexcept = default;

        void clearChildren() override { files.clear(); cache.stats.reset(); }
        void clear();
        /// @return  maybe alias-constructed s_p, but never null
        std::shared_ptr<Project> self();        
        std::shared_ptr<UiObject> selfUi() override { return self(); }

        ObjType objType() const noexcept override { return ObjType::PROJECT; }
        std::shared_ptr<File> file() override { return {}; }
        size_t nChildren() const noexcept override { return files.size(); };
        std::shared_ptr<Entity> child(size_t i) const override;
        std::shared_ptr<UiObject> parent() const override { return {}; }
        std::u8string_view idColumn() const override { return {}; }
        std::shared_ptr<Project> project() override { return self(); }
        Pair<VirtualGroup> additionParents() override { return {}; }
        std::shared_ptr<Entity> extractChild(size_t i, Modify wantModify) override;
        void writeToXml(
                pugi::xml_node&,
                const std::filesystem::path& basePath) const;
        bool unmodify(Forced forced) override;
        void traverse(TraverseListener& x, tr::WalkOrder order, EnterMe enterMe) override;
        std::shared_ptr<VirtualGroup> nearestGroup() override { return {}; }
        void removeTranslChannel() override;

        void save();
        void save(const std::filesystem::path& aFname);
        void saveCopy(const std::filesystem::path& aFname) const;
        void readFromXml(
                const pugi::xml_node& node,
                const std::filesystem::path& basePath);
        void load(
                const pugi::xml_document& doc,
                const std::filesystem::path& basePath);
        void load(const std::filesystem::path& aFname);
        void doBuild(const std::filesystem::path& destDir);
        WalkChannel walkChannel() const;

        // Adds a file in the end of project
        std::shared_ptr<File> addFile(
                std::u8string_view name, Modify);
        /// @return  # of exportable files in ORIGINAL mode
        /// @warning  In TRANSLATION mode original IDs/texts should be OK
        size_t nOrigExportableFiles() const;

        std::u8string shownFname(std::u8string_view fallback);

        UpdateInfo updateData();
        tr::UpdateInfo stealDataFrom(tr::Project& x);
        std::shared_ptr<tr::File> findFile(std::u8string_view aId);

        /// Use instead of ctor
        /// @warning  Refer to private ctors to see which versions are available
        template<class... T>
            static std::shared_ptr<tr::Project> make(T&& ... x);

        /// Passkey idiom, used by make
        /// @warning  Do not call directly
        template <class... T>
            Project(const PassKey&, T&&... x)
                : Project(std::forward<T>(x)...) {}
    protected:
        void doSwapChildren(size_t index1, size_t index2) override;
    private:
        /// Ctors are private, use make!
        Project() = default;
        Project(const Project&) = default;
        Project(PrjInfo&& aInfo) noexcept : info(std::move(aInfo)) {}
        void doShare(const std::shared_ptr<Project>& x);
        UpdateInfo updateData_FullTransl();
    };

}   // namespace tr


///// Template implementations /////////////////////////////////////////////////


template <tr::ObjType Objt>
std::u8string tr::UiObject::makeId(const IdLib* idlib, const UiObject* src) const
{
    if (idlib) {
        return makeId<Objt>(*idlib);
    } else if (src) {
        return std::u8string { src->idColumn() };
    } else {
        return std::u8string { idColumn() };
    }
}

template <class T>
unsigned tr::Pair<T>::size() const
{
    if (!first)
        return 0;
    return second ? 2 : 1;
}

template<class... T>
    std::shared_ptr<tr::Project> tr::Project::make(T&& ... x)
{
    auto r = std::make_shared<tr::Project>(PassKey{}, std::forward<T>(x) ...);
    r->doShare(r);
    return r;
}
