#pragma once

// STL
#include <vector>
#include <memory>
#include <atomic>
#include <optional>
#include <filesystem>

// Translator
#include "TrFile.h"
#include "TrDefines.h"

namespace tr {

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
        Self& operator = (const Self&) { return *this; }
        std::weak_ptr<T> fSelf;
    };

    class UiObject
    {
    public:
        struct Cache {
            int index = -1;
            bool isExpanded = false;
            //bool isDeleted = false;
        } cache;
        UiObject();
        virtual ~UiObject();

        virtual ObjType type() const = 0;
        virtual std::shared_ptr<UiObject> parent() const = 0;
        virtual size_t nChildren() const = 0;
        virtual std::shared_ptr<UiObject> child(size_t i) const = 0;
        virtual std::u8string_view idColumn() const = 0;
        /// @todo [architecture] How to invent parallel VMT?
        virtual std::u8string_view origColumn() const { return {}; }
        virtual std::u8string_view translColumn() const { return {}; }

        void checkCanary() const;
        void recache();
        void recursiveRecache();

        // Do nothing: cache is cache, and canary depends on pointer
        UiObject(const UiObject&) : UiObject() {}
        UiObject(UiObject&&) noexcept : UiObject() {}
        UiObject& operator=(const UiObject&) { return *this; }
        UiObject& operator=(UiObject&&) noexcept { return *this; }
    protected:
        std::atomic<uint32_t> canary  = 0;

        uint32_t goodCanary() const;
        // passkey idiom
        struct Key {};
    };

    class Entity : public UiObject
    {
    public:
        std::u8string id,               ///< Identifier of group or string
                      authorsComment;   ///< Author’s comment

        std::u8string_view idColumn() const override { return id; }
    };

    class VirtualGroup : public Entity, private Self<VirtualGroup>
    {
    private:
        using Super = Entity;
    public:
        std::vector<std::shared_ptr<UiObject>> children;

        // New virtual
        virtual std::shared_ptr<File> file() = 0;

        size_t nChildren() const override { return children.size(); };
        std::shared_ptr<UiObject> child(size_t i) const override;

        using Super::Super;

        std::shared_ptr<Text> addText(std::u8string id, std::u8string original);
        std::shared_ptr<Group> addGroup(std::u8string id);
    protected:
        friend class Project;
    };

    class Text final : public Entity
    {
    public:
        std::u8string original;     ///< Current original string
        std::optional<std::u8string>
                    knownOriginal,  ///< Known original string we translated
                    translation;    ///< Translation for known original (if present) or original
        std::u8string translatorsComment;   ///< Translator’s comment

        ObjType type() const override { return ObjType::TEXT; }
        size_t nChildren() const override { return 0; };
        std::shared_ptr<UiObject> child(size_t) const override { return {}; }
        std::u8string_view origColumn() const override { return original; }
        std::u8string_view translColumn() const override
            { return translation.has_value() ? *translation : std::u8string_view{}; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }

        Text(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const Key&);
    private:
        std::weak_ptr<VirtualGroup> fParentGroup;
    };

    class Group final : public VirtualGroup
    {
    private:
        using Super = VirtualGroup;
    public:
        std::unique_ptr<tf::FileInfo> linkedFile;

        ObjType type() const override { return ObjType::GROUP; }
        std::shared_ptr<UiObject> parent() const override { return fParentGroup.lock(); }
        std::shared_ptr<File> file() override { return fFile.lock(); }
        Group(std::weak_ptr<VirtualGroup> aParent, size_t aIndex, const Key&);
    private:
        friend class tr::File;
        std::weak_ptr<File> fFile;
        std::weak_ptr<VirtualGroup> fParentGroup;
    };

    class File final : public VirtualGroup
    {
    public:
        std::shared_ptr<Project> project() { return fProject.lock(); }
        std::unique_ptr<tf::FileInfo> fileInfo;

        ObjType type() const override { return ObjType::FILE; }
        std::shared_ptr<UiObject> parent() const override;
        std::shared_ptr<File> file() override
            { return std::shared_ptr<File>{this}; }

        File(std::weak_ptr<Project> aProject, size_t aIndex, const Key&);
    protected:
        std::weak_ptr<Project> fProject;
    };

    class Project final : public UiObject, private Self<Project>
    {
    public:
        PrjInfo info;
        std::filesystem::path fname;
        std::vector<std::shared_ptr<File>> files;

        /// @brief addTestOriginal
        ///   Adds a few files and strings that will serve as test original
        ///   (project will be original only, w/o translation)
        void addTestOriginal();

        Project() = default;
        Project(PrjInfo&& aInfo) noexcept : info(std::move(aInfo)) {}

        void clear();
        void doShare(const std::shared_ptr<Project>& x) { fSelf = x; }
        std::shared_ptr<tr::Project> self();

        ObjType type() const override { return ObjType::PROJECT; }
        size_t nChildren() const override { return files.size(); };
        std::shared_ptr<UiObject> child(size_t i) const override;
        std::shared_ptr<UiObject> parent() const override { return {}; }
        std::u8string_view idColumn() const override { return {}; }

        // Adds a file in the end of project
        std::shared_ptr<File> addFile();
        std::shared_ptr<File> addFile(std::u8string_view name);
    };

}   // namespace tr
