#pragma once

// Qt
#include <QString>

// STL
#include <span>

namespace tr {

    class Loader     // interface
    {
    public:
        /// Adds group, goes to it
        virtual void addGroup(const QString& id) = 0;
        /// Goes one group up
        virtual void goUp() = 0;
        /// Adds text at relative position, DOES NOT go to newly-created groups
        virtual void addTextAtRel(
                std::span<const QString> ids,
                const QString& original,
                const QString& comment) = 0;
        /// Virtual dtor
        virtual ~Loader() = default;

        // Utils
        virtual void addText(const QString& id, const QString& original, const QString& comment)
            { addTextAtRel(std::span<const QString>(&id, 1), original, comment); }
    };

    class Walker
    {
    public:
        virtual ~Walker() = default;
        virtual bool nextGroup() = 0;
        virtual bool enter() = 0;
        virtual void leave() = 0;
        virtual bool nextText() = 0;
        virtual QString id() = 0;
        virtual QString idChain(QChar separator) = 0;
        /// @warning  Right now we export single-language data only,
        ///           but DO NOT use for dual-language one!
        ///           Use original() and translation() instead!
        virtual QString text() = 0;
        virtual QString original() = 0;
        virtual QString translation() = 0;
        /// @return [+] id(), idChain(), text(),
        ///         and in the future original() and translation()
        ///         will temporarily show info on what we found
        ///         [-] not found, nothing happens
        virtual bool findTextAtRel(std::span<const QString> ids) = 0;

        // Utils
        virtual bool findText(const QString& id)
            { return findTextAtRel(std::span<const QString>(&id, 1)); }
    };

    class FileInfo      // interface
    {
    public:
        virtual void doImport(Loader& loader) = 0;
        virtual void doExport(Walker& walker) = 0;

        ///
        /// \return [+] needs file and cannot export if it’s absent (e.g. Qt form)
        ///         [−] creates file from scratch (e.g. simple text/binary file, Transifex XLIFF)
        ///
        virtual bool needsFile() const = 0;
        virtual ~FileInfo() = default;
    };

}   // namespace tr
