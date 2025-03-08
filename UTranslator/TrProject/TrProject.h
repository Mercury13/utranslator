#pragma once

// STL
#include <memory>
#include <optional>
#include <filesystem>

// Translator
#include "TrDefines.h"
#include "TrVirtuals.h"
#include "Modifiable.h"

// Libs
#include "u_Vector.h"
#include "u_Strings.h"

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
    static constexpr int N = 3;
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

    class VirtualGroup;
    class Group;
    class File;
    class Project;
    class Text;
    class UiObject;

    enum class ObjType : unsigned char { PROJECT, FILE, GROUP, TEXT };

    enum class ObjState : unsigned char { STAYING, ADDED, DELETED };

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

    /// Simple cache to speed up writing
    struct WrCache {
        const PrjInfo& info;
        std::filesystem::path baseDir;
        std::u8string u8;

        WrCache(const PrjInfo& aInfo, std::filesystem::path aBaseDir)
            : info(aInfo), baseDir(std::move(aBaseDir)) {}

        /// Ensures UTF-8 string length + 8 additional bytes
        void ensureU8(size_t length);
        /// Turns beg..end to null-terminated string
        const char8_t* nts(const char8_t* beg, const char8_t* end);
        const char* ntsC(const char8_t* beg, const char8_t* end)
            { return reinterpret_cast<const char*>(nts(beg, end)); }

        std::filesystem::path toRelPath(const std::filesystem::path& path)
            { return std::filesystem::proximate(path, baseDir); }
    };

    struct UiFileInfo {
        tr::FileInfo* info;
        tf::ProtoFilter filter;
    };

    struct StealContext {
        tf::StealOrig orig;
    };

    struct ReadContext {
        const PrjInfo& info;
        std::filesystem::path baseDir;

        std::filesystem::path toAbsPath(const std::filesystem::path& x) const;
        std::filesystem::path toAbsPath(std::string_view x) const
            { return toAbsPath(str::toU8sv(x)); }
        std::filesystem::path toAbsPath(const char* x) const
            { return toAbsPath(str::toU8sv(x)); }
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
        virtual void readFromXml(const pugi::xml_node& node, const ReadContext& ctx) = 0;
        virtual std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>& parent) const = 0;
    protected:
        friend class VirtualGroup;
        virtual void updateParent(const std::shared_ptr<VirtualGroup>& x) = 0;
        // write comments
        void writeImportersAuthorsComment(pugi::xml_node& node, WrCache& c) const;
        void writeTranslatorsComment(pugi::xml_node& node, WrCache& c) const;
        void writeComments(pugi::xml_node& node, WrCache&) const;
        // read comments
        void readAuthorsComment(const pugi::xml_node& node);
        void readTranslatorsComment(const pugi::xml_node& node, const PrjInfo& info);
        void readComments(const pugi::xml_node& node, const PrjInfo& info);
        void entityRemoveTranslChannel();
        void entityStealDataFrom(Entity& x, UiObject* myParent, const StealContext& ctx);
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
        void collectSyncGroups(std::vector<std::shared_ptr<tr::Group>>& r);
        void markChildrenAsAddedToday() override;
        void traverseTexts(const EvText&) override;
        void traverseCTexts(const EvCText&) const override;
    protected:
        friend class Project;
        void doSwapChildren(size_t index1, size_t index2) override;

        void writeCommentsAndChildren(pugi::xml_node&, WrCache&) const;
        void readCommentsAndChildren(const pugi::xml_node& node, const ReadContext& ctx);
        void vgRemoveTranslChannel();
        tr::UpdateInfo vgStealDataFrom(
                VirtualGroup& x, UiObject* myParent, const StealContext& ctx);
        void vgStealReferenceFrom(VirtualGroup& x);
        void vgUpdateChildrensParents(const std::shared_ptr<VirtualGroup>& that);
    };

    class Text final : public Entity, protected Self<Text>
    {
    public:
        Translatable tr;

        Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const PassKey&);

        ObjType objType() const noexcept override { return ObjType::TEXT; }
        size_t nChildren() const noexcept override { return 0; };
        std::shared_ptr<Entity> child(size_t) const override { return {}; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        Pair<VirtualGroup> additionParents() override { return fParentGroup.lock(); }
        Translatable* translatable() override { return &tr; }
        std::shared_ptr<File> file() override;
        std::shared_ptr<Project> project() override;
            using Entity::project;
        void writeToXml(pugi::xml_node&, WrCache&) const override;
        void readFromXml(const pugi::xml_node& node, const ReadContext& ctx) override;
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
        void markChildrenAsAddedToday() override { tr.wasChangedToday = true; }
        void traverseTexts(const EvText&) override;
        void traverseCTexts(const EvCText&) const override;
        ///  @return  CHANGED data
        UpdateInfo::ByState stealDataFrom(
                Text& x, UiObject* myParent, const StealContext& ctx);
        void stealReferenceFrom(Text& x);
    protected:
        std::shared_ptr<Entity> vclone(
                const std::shared_ptr<VirtualGroup>& parent) const override
            { return clone(parent, nullptr, Modify::NO); }
        void updateParent(const std::shared_ptr<VirtualGroup>& x) override;
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
        struct Sync {
            std::unique_ptr<tf::FileFormat> format;
            std::filesystem::path absPath;
            tf::SyncInfo info;
            explicit operator bool() const { return static_cast<bool>(format); }
        } sync;

        Group(const std::shared_ptr<VirtualGroup>& aParent,
              size_t aIndex, const PassKey&);

        ObjType objType() const noexcept override { return ObjType::GROUP; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        std::shared_ptr<File> file() override { return fFile.lock(); }
        Pair<VirtualGroup> additionParents() override
                { return { fParentGroup.lock(), fSelf.lock() }; }
        void writeToXml(pugi::xml_node&, WrCache&) const override;
        void readFromXml(const pugi::xml_node& node, const ReadContext& ctx) override;
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
        UpdateInfo stealDataFrom(Group& x, UiObject* myParent, const StealContext& ctx)
            { return vgStealDataFrom(x, myParent, ctx); }
        void stealReferenceFrom(Group& x) { return vgStealReferenceFrom(x); }
        UpdateInfo updateData();
    protected:
        void updateParent(const std::shared_ptr<VirtualGroup>& x) override;
    private:
        friend class tr::File;
        std::weak_ptr<File> fFile;
        std::weak_ptr<VirtualGroup> fParentGroup;

        UpdateInfo updateData_Original();
    };

    enum class FileMode : unsigned char { HOSTED, EXTERNAL };

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
        void readFromXml(const pugi::xml_node& node, const ReadContext& ctx) override;
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
        UpdateInfo stealDataFrom(File& x, UiObject* myParent, const StealContext& ctx);
        void stealReferenceFrom(File& x);
        void updateParents(const std::shared_ptr<Project>& x);
    protected:
        std::weak_ptr<Project> fProject;
        /// actually unused. Maybe a bit poor architecture…
        virtual void updateParent(const std::shared_ptr<VirtualGroup>&) override {}
    };

    enum class TrashMode : unsigned char { LEAVE, FILL };

    class Project final :
            public UiObject,
            public SimpleModifiable,
            private Self<Project>
    {
    public:
        PrjInfo info;
        std::filesystem::path fname;
        SafeVector<std::shared_ptr<File>> files;
        Trash trash;

        /// @brief addTestOriginal
        ///   Adds a few files and strings that will serve as test original
        ///   (project will be original only, w/o translation)
        void addTestOriginal();

        /// Ctors are private, op= is the same
        Project& operator = (const Project&) = delete;
        Project& operator = (Project&&) noexcept = default;

        void clearChildren() override { files.clear(); cache.stats.reset(); }
        void clear();
        std::shared_ptr<UiObject> selfUi() override { return fSelf.lock(); }

        ObjType objType() const noexcept override { return ObjType::PROJECT; }
        std::shared_ptr<File> file() override { return {}; }
        size_t nChildren() const noexcept override { return files.size(); };
        std::shared_ptr<Entity> child(size_t i) const override;
        std::shared_ptr<UiObject> parent() const override { return {}; }
        std::u8string_view idColumn() const override { return {}; }
        std::shared_ptr<Project> project() override { return fSelf.lock(); }
        Pair<VirtualGroup> additionParents() override { return {}; }
        std::shared_ptr<Entity> extractChild(size_t i, Modify wantModify) override;
        void writeToXml(
                pugi::xml_node&,
                const std::filesystem::path& basePath) const;
        bool unmodify(Forced forced) override;
        void traverse(TraverseListener& x, tr::WalkOrder order, EnterMe enterMe) override;
        std::shared_ptr<VirtualGroup> nearestGroup() override { return {}; }
        void removeTranslChannel() override;
        void markChildrenAsAddedToday() override;
        void traverseTexts(const EvText&) override;
        void traverseCTexts(const EvCText&) const override;
        void updateParents();

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

        UpdateInfo updateData(TrashMode mode);
        void updateReference();
        tr::UpdateInfo stealDataFrom(tr::Project& x, const StealContext& ctx);
        void stealReferenceFrom(tr::Project& x);
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

        std::vector<std::shared_ptr<Group>> syncGroups();
    protected:        
        void doSwapChildren(size_t index1, size_t index2) override;
    private:
        /// Ctors are private, use make!
        Project() = default;
        Project(const Project&) = delete;
        Project(Project&&) = default;
        Project(PrjInfo&& aInfo) noexcept : info(std::move(aInfo)) {}
        UpdateInfo updateData_FullTransl(TrashMode mode);
    };

    ///  To prevent TrFinder from including everywhere
    class FindCriterion     // interface
    {
    public:
        virtual bool matchText(const tr::Text&) const = 0;
        virtual bool matchGroup(const tr::VirtualGroup&) const { return false; }
        virtual std::u8string caption() const = 0;
        virtual ~FindCriterion() = default;
    };


    template <tr::ObjType Objt>
    inline std::u8string makeIdT(const tr::UiObject& obj, const tr::IdLib& idlib)
    {
        if constexpr (Objt == tr::ObjType::PROJECT) {
            return u8"project";
        } else if constexpr (Objt == tr::ObjType::FILE) {
            return obj.makeId(idlib.filePrefix, idlib.fileSuffix);
        } else if constexpr (Objt == tr::ObjType::GROUP) {
            return obj.makeId(idlib.groupPrefix, {});
        } else if constexpr (Objt == tr::ObjType::TEXT) {
            return obj.makeTextId(idlib);
        } else {
            throw std::logic_error("[UiObject.makeId] Strange objType");
        }
    }

}   // namespace tr


///// Template implementations /////////////////////////////////////////////////


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
    r->fSelf = r;
    return r;
}

