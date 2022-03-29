#pragma once

// STL
#include <memory>
#include <atomic>
#include <optional>
#include <filesystem>

// Translator
#include "TrFile.h"
#include "TrDefines.h"
#include "Modifiable.h"

// Libs
#include "u_Vector.h"

namespace pugi {
    class xml_node;
}

namespace tr {

    /// Modification channel
    enum class Mch {
        ID      = 1,
        ORIG    = 2,
        TRANSL  = 4,
        COMMENT = 8   ///< as we do not show comments in table, let it be…
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

    enum class ObjType { PROJECT, FILE, GROUP, TEXT };

    class Entity;

    template <class T>
    class Self
    {
    public:
        Self() = default;
        Self(const Self&) {}
        Self(Self&&) {}
        Self& operator = (const Self&) { return *this; }
        Self& operator = (Self&&) { return *this; }
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
        std::atomic<uint32_t> canary  = 0;
        uint32_t goodCanary() const;
    };

    struct Comments {
        std::u8string authors, translators;
    };

    struct Translatable {
        std::u8string original;     ///< Current original string
        std::optional<std::u8string>
                    knownOriginal,  ///< Known original string we translated
                    translation;    ///< Translation for known original (if present) or original
        std::u8string_view translationSv() const
            { return translation ? *translation : std::u8string_view(); }
    };

    enum class Modify { NO, YES };

    class UiObject : public CanaryObject
    {
    public:
        struct Cache {
            int index = -1;             ///< index in tree
            //bool isExpanded = false;    ///< [+] was expanded in tree; unused right now
            //bool isDeleted = false;     ///< [+] deleted from original and left for history; unused right now
            Mod mod;
        } cache;
        // Just here we use virtual dtor!
        virtual ~UiObject() = default;

        virtual ObjType objType() const = 0;
        virtual std::shared_ptr<UiObject> parent() const = 0;
        virtual size_t nChildren() const = 0;
        virtual std::shared_ptr<Entity> child(size_t i) const = 0;
        virtual std::u8string_view idColumn() const = 0;
        /// @todo [architecture] How to invent parallel VMTs?
        virtual std::u8string_view origColumn() const { return {}; }
        virtual std::u8string_view translColumn() const { return {}; }
        /// @return [+] was changed
        virtual bool setId(std::u8string_view, tr::Modify) { return false; }
        /// Deletes i’th child
        /// @return extracted child
        /// @warning  Because of recache, complexity is O(n)
        virtual std::shared_ptr<Entity> extractChild(size_t) { return {}; }

        /// @return  ptr to comments, or null
        virtual Comments* comments() { return nullptr; }
        /// @return  ptr to original/translation, or null
        virtual Translatable* translatable() { return nullptr; }
        /// @return  ptr to project
        virtual std::shared_ptr<Project> project() = 0;
        /// @return  one or two parent groups for “Add group” / “Add string”
        virtual Pair<VirtualGroup> additionParents() = 0;

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
        /// @return some ID
        std::u8string makeId(
                std::u8string_view prefix,
                std::u8string_view suffix) const;
        /// @return  [+] s_p to this  [0] nothing happened
        std::shared_ptr<Entity> extract();
        /// @return  how many texts are there in object’s groups
        virtual size_t nTexts() const;
    protected:
        // passkey idiom
        struct PassKey {};
        void doModify(Mch ch);
    };

    class Entity : public UiObject
    {
    public:
        std::u8string id;               ///< Identifier of group or string
        Comments comm;

        std::u8string_view idColumn() const override { return id; }
        Comments* comments() override { return &comm; }
        bool setId(std::u8string_view x, tr::Modify wantModify) override;

        // New virtuals
        virtual std::shared_ptr<File> file() = 0;

        /// Writes object to XML
        /// @param [in,out] root  tag ABOVE, should create a new one for entity
        /// @param [in] info   project info for speed
        virtual void writeToXml(pugi::xml_node& root, const PrjInfo& info) const = 0;
    protected:
        void writeAuthorsComment(pugi::xml_node& node) const;
        void writeTranslatorsComment(pugi::xml_node& node, const PrjInfo& info) const;
        void writeComments(pugi::xml_node& node, const PrjInfo& info) const;
    };

    class VirtualGroup : public Entity, protected Self<VirtualGroup>
    {
    private:
        using Super = Entity;
    public:
        SafeVector<std::shared_ptr<Entity>> children;

        size_t nChildren() const override { return children.size(); };
        std::shared_ptr<Entity> child(size_t i) const override;
        std::shared_ptr<Entity> extractChild(size_t i) override;

        using Super::Super;

        std::shared_ptr<Text> addText(
                std::u8string id, std::u8string original,
                Modify wantModify);
        std::shared_ptr<Group> addGroup(
                std::u8string id, Modify wantModify);
        std::shared_ptr<Project> project() override;
    protected:
        friend class Project;
        void writeCommentsAndChildren(pugi::xml_node&, const PrjInfo&) const;
    };

    class Text final : public Entity
    {
    public:
        Translatable tr;

        Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const PassKey&);

        ObjType objType() const override { return ObjType::TEXT; }
        size_t nChildren() const override { return 0; };
        std::shared_ptr<Entity> child(size_t) const override { return {}; }
        std::u8string_view origColumn() const override { return tr.original; }
        std::u8string_view translColumn() const override
            { return tr.translation.has_value() ? *tr.translation : std::u8string_view{}; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        Pair<VirtualGroup> additionParents() override { return fParentGroup.lock(); }
        Translatable* translatable() override { return &tr; }
        std::shared_ptr<File> file() override;
        std::shared_ptr<Project> project() override;
        size_t nTexts() const override { return 1; }
        void writeToXml(pugi::xml_node&, const PrjInfo&) const override;
    private:
        std::weak_ptr<VirtualGroup> fParentGroup;
    };

    class Group final : public VirtualGroup
    {
    private:
        using Super = VirtualGroup;
    public:
        std::unique_ptr<tf::FileInfo> linkedFile;

        Group(const std::shared_ptr<VirtualGroup>& aParent,
              size_t aIndex, const PassKey&);

        ObjType objType() const override { return ObjType::GROUP; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        std::shared_ptr<File> file() override { return fFile.lock(); }
        Pair<VirtualGroup> additionParents() override
                { return { fParentGroup.lock(), fSelf.lock() }; }
        void writeToXml(pugi::xml_node&, const PrjInfo&) const override;
    private:
        friend class tr::File;
        std::weak_ptr<File> fFile;
        std::weak_ptr<VirtualGroup> fParentGroup;
    };

    class File final : public VirtualGroup
    {
    public:
        std::shared_ptr<Project> project() override { return fProject.lock(); }
        std::unique_ptr<tf::FileInfo> fileInfo;

        ObjType objType() const override { return ObjType::FILE; }
        std::shared_ptr<UiObject> parent() const override;
        std::shared_ptr<File> file() override
            { return std::dynamic_pointer_cast<File>(fSelf.lock()); }
        Pair<VirtualGroup> additionParents() override { return fSelf.lock(); }

        File(std::weak_ptr<Project> aProject, size_t aIndex, const PassKey&);
        virtual void writeToXml(pugi::xml_node&, const PrjInfo&) const override;
    protected:
        std::weak_ptr<Project> fProject;
    };

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

        Project() = default;
        Project(PrjInfo&& aInfo) noexcept : info(std::move(aInfo)) {}

        void clear();
        void doShare(const std::shared_ptr<Project>& x);
        /// @return  maybe alias-constructed s_p, but never null
        std::shared_ptr<Project> self();

        ObjType objType() const override { return ObjType::PROJECT; }
        size_t nChildren() const override { return files.size(); };
        std::shared_ptr<Entity> child(size_t i) const override;
        std::shared_ptr<UiObject> parent() const override { return {}; }
        std::u8string_view idColumn() const override { return {}; }
        std::shared_ptr<Project> project() override { return self(); }
        Pair<VirtualGroup> additionParents() override { return {}; }
        std::shared_ptr<Entity> extractChild(size_t i) override;
        void writeToXml(pugi::xml_node&) const;
        bool unmodify() override;
        void save();
        void save(const std::filesystem::path& aFname);
        void saveCopy(const std::filesystem::path& aFname) const;

        // Adds a file in the end of project
        std::shared_ptr<File> addFile(
                std::u8string_view name, Modify);
    };

}   // namespace tr


///// Template implementations /////////////////////////////////////////////////


template <class T>
unsigned tr::Pair<T>::size() const
{
    if (!first)
        return 0;
    return second ? 2 : 1;
}
