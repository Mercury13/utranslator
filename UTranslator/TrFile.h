#pragma once

// Qt
#include <QString>

// STL
#include <span>

namespace tr {

    class VirtualLoader     // interface
    {
    public:
        /// Adds group, goes to it
        virtual void addGroup(const QString& id) = 0;
        /// Goes one group up
        virtual void goUp() = 0;
        /// Adds text at relative position, DOES NOT go to newly-created groups
        virtual void addTextAtRel(std::span<const QString> ids, const QString& original) = 0;
        /// Virtual dtor
        virtual ~VirtualLoader() = default;

        // Utils
        virtual void addText(const QString& id, const QString& original)
            { addTextAtRel(std::span<const QString>(&id, 1), original); }
    };

    class FileInfo      // interface
    {
        virtual void load(VirtualLoader& loader) = 0;
        virtual bool needsOriginalFile() const = 0;
        virtual ~FileInfo() = default;
    };

}   // namespace tr
