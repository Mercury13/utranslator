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

    enum class ObjType { PROJECT, FILE, GROUP, TEXT };

    class Entity;

    class UiObject : public std::enable_shared_from_this<UiObject>
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

        // Do nothing: temps are temps, and canary depends on pointer
        UiObject(const UiObject&) : UiObject() {}
        UiObject& operator=(const UiObject&) { return *this; }
    protected:
        std::atomic<uint32_t> canary  = 0;

        uint32_t goodCanary() const;
    };

    class Entity : public UiObject
    {
    public:
        std::u8string id,               ///< Identifier of group or string
                      authorsComment;   ///< Author’s comment
        std::shared_ptr<Group> parentGroup() const { return fParentGroup.lock(); }
        std::shared_ptr<File> file() const { return fFile.lock(); }
        Entity(std::weak_ptr<Group> aGroup, std::weak_ptr<File> aFile)
            : fParentGroup(std::move(aGroup)), fFile(std::move(aFile)) {}

        std::u8string_view idColumn() const override { return id; }
        std::shared_ptr<UiObject> parent() const override;
    protected:
        std::weak_ptr<Group> fParentGroup;
        std::weak_ptr<File> fFile;
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
    };

    class Group : public Entity
    {
    private:
        // passkey idiom
        struct Key {};
    public:
        std::vector<std::shared_ptr<UiObject>> children;
        std::unique_ptr<tf::FileInfo> linkedFile;
        Group(File& owner, const Key&) : Entity({}, std::shared_ptr<File>(&owner)) {}

        ObjType type() const override { return ObjType::TEXT; }
        size_t nChildren() const override { return children.size(); };
        std::shared_ptr<UiObject> child(size_t i) const override;
    private:
        friend class tr::File;
    };

    class File final : public Group
    {
    public:
        std::shared_ptr<Project> project() { return fProject.lock(); }
        std::unique_ptr<tf::FileInfo> fileInfo;

        ObjType type() const override { return ObjType::FILE; }
        std::shared_ptr<UiObject> parent() const override;
    protected:
        std::weak_ptr<Project> fProject;
    };

    class Project final : public UiObject
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
        Project(PrjInfo&& aInfo) : info(std::move(aInfo)) {}
        void clear();

        ObjType type() const override { return ObjType::PROJECT; }
        size_t nChildren() const override { return files.size(); };
        std::shared_ptr<UiObject> child(size_t i) const override;
        std::shared_ptr<UiObject> parent() const override { return {}; }
        std::u8string_view idColumn() const override { return {}; }
    };

}   // namespace tr
