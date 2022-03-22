#pragma once

// Qt
#include <QString>

// STL
#include <span>

// Libs
#include "u_TypedFlags.h"


namespace tf {

    class Loader     // interface
    {
    public:
        /// Adds group, GOES to it
        virtual void addGroup(const QString& id) = 0;
        /// Goes one group up
        virtual void goUp() = 0;
        /// Adds text at relative position, DOES NOT go to newly-created groups
        /// @param [in] ids   relative path, often one id
        virtual void addTextAtRel(
                std::span<const QString> ids,
                const QString& original,
                const QString& comment) = 0;
        /// Virtual dtor
        virtual ~Loader() = default;

        // Utils
        void addText(const QString& id, const QString& original, const QString& comment)
            { addTextAtRel(std::span<const QString>(&id, 1), original, comment); }
    };

    class Walker    // interface
    {
    public:
        virtual ~Walker() = default;
        /// Groups and texts are mutually-exclusive things
        virtual bool nextGroup() = 0;
        virtual bool enter() = 0;
        virtual void leave() = 0;
        virtual bool nextText() = 0;
        virtual QString id() = 0;
        virtual QString idChain(QChar separator) = 0;
        /// @warning  DO NOT use for dual-language data!
        ///           Use original() and translation() instead.
        virtual QString text() = 0;
        /// @warning  DO NOT use for single-language data!
        ///           Use text() instead.
        virtual QString original() = 0;
        virtual QString translation() = 0;
        /// @return [+] id(), idChain(), text(), original(), translation()
        ///         will temporarily show info on what we found
        ///         [-] not found, nothing happens
        virtual bool findTextAtRel(std::span<const QString> ids) = 0;

        // Utils
        bool findText(const QString& id)
            { return findTextAtRel(std::span<const QString>(&id, 1)); }
    };

    enum class Fcap {
        IMPORT = 1,
        EXPORT = 2,
        /// [+] needs file and cannot export if it’s absent (e.g. Qt form)
        /// [−] creates file from scratch (e.g. simple text/binary file, Transifex XLIFF)
        NEEDS_FILE = 4,
    };
    DEFINE_ENUM_OPS(Fcap)

    ///
    /// \brief The FileInfo class
    ///   Common ancestor for file import/export
    ///
    class FileInfo
    {
    public:
        // do not want to extract interface, as we work with files only
        QString fname;

        virtual void doImport(Loader& loader) = 0;
        virtual void doExport(Walker& walker) = 0;

        virtual Flags<Fcap> caps() const noexcept = 0;
        virtual ~FileInfo() = default;
    };

    ///
    /// \brief The EnumText class
    ///   Simple type of file:
    ///     This is string one        << id=1
    ///     And this string two       << id=2
    ///     String three              << id=3
    ///
    class EnumText final : public FileInfo
    {
        void doImport(Loader& loader) override;
        void doExport(Walker&) override {}

        /// @todo [urgent] can export too, but let’s import somehow
        Flags<Fcap> caps() const noexcept override { return Fcap::IMPORT; }
    };

}   // namespace tr
