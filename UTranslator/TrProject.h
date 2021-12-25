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
        struct Temp {
            bool isExpanded;
        } temp;
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
    public:
        std::vector<std::shared_ptr<Group>> groups;
        std::vector<std::shared_ptr<Text>> texts;
    };

    class File : public UiObject
    {
    public:
        std::shared_ptr<Project> project() { return fProject.lock(); }
    protected:
        std::weak_ptr<Project> fProject;
    };

    enum class PrjType { ORIGINAL, TRANSLATION };

    struct Project
    {
        PrjType type;
        std::vector<std::shared_ptr<File>> files;
    };

}   // namespace tr
