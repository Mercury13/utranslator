#pragma once

// Qt
#include <QString>

// STL
#include <vector>
#include <memory>
#include <atomic>

namespace tr {

    class Group;
    class File;
    struct Project;

    class UiObject : public std::enable_shared_from_this<UiObject>
    {
    public:
        struct Cache {
            int index = -1;
            bool isExpanded = false;
            bool isDeleted = false;
        } cache;
        UiObject();
        ~UiObject();
        void checkCanary() const;

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
        QString id, comment;
        std::shared_ptr<Group> group() { return fGroup.lock(); }
        std::shared_ptr<File> file() { return fFile.lock(); }
        Entity(std::weak_ptr<Group> aGroup, std::weak_ptr<File> aFile)
            : fGroup(std::move(aGroup)), fFile(std::move(aFile)) {}
    protected:
        std::weak_ptr<Group> fGroup;
        std::weak_ptr<File> fFile;
    };

    class Text : public Entity
    {
    public:
        QString original, known, translation, translatorsComment;
    };

    class Group : public Entity
    {
    private:
        // passkey idiom
        struct Key {};
    public:
        std::vector<std::shared_ptr<Group>> groups;
        std::vector<std::shared_ptr<Text>> texts;
        Group(File& owner, const Key&) : Entity({}, std::shared_ptr<File>(&owner)) {}
    private:
        friend class tr::File;
    };

    class File : public UiObject
    {
    public:
        std::shared_ptr<Project> project() { return fProject.lock(); }
        Group& root() { return *fRoot; }
        const Group& root() const { return *fRoot; }
    protected:
        std::weak_ptr<Project> fProject;
        std::shared_ptr<Group> fRoot = std::make_shared<Group>(*this, Group::Key());
    };

    enum class PrjType { ORIGINAL, TRANSLATION };

    struct Project : public UiObject
    {
        PrjType type = PrjType::ORIGINAL;
        std::vector<std::shared_ptr<File>> files;
    };

}   // namespace tr
