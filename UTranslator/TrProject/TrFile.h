#pragma once

// STL
#include <string>
#include <span>

#include "TrDefines.h"

namespace tf {

    class Loader     // interface
    {
    public:
        /// Adds group, GOES to it
        virtual void addGroup(const std::u8string& id) = 0;
        /// Goes one group up
        virtual void goUp() = 0;
        /// Adds text at relative position, DOES NOT go to newly-created groups
        /// @param [in] ids   relative path, often one id
        virtual void addTextAtRel(
                std::span<const std::u8string> ids,
                const std::u8string& original,
                const std::u8string& comment) = 0;
        /// Virtual dtor
        virtual ~Loader() = default;

        // Utils
        void addText(const std::u8string& id, const std::u8string& original, const std::u8string& comment)
            { addTextAtRel(std::span<const std::u8string>(&id, 1), original, comment); }
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
        virtual std::u8string id() = 0;
        virtual std::u8string idChain(char separator) = 0;
        /// @warning  DO NOT use for dual-language data!
        ///           Use original() and translation() instead.
        virtual std::u8string text() = 0;
        /// @warning  DO NOT use for single-language data!
        ///           Use text() instead.
        virtual std::u8string original() = 0;
        virtual std::u8string translation() = 0;
        /// @return [+] id(), idChain(), text(), original(), translation()
        ///         will temporarily show info on what we found
        ///         [-] not found, nothing happens
        virtual bool findTextAtRel(std::span<const std::u8string> ids) = 0;

        // Utils
        bool findText(const std::u8string& id)
            { return findTextAtRel(std::span<const std::u8string>(&id, 1)); }
    };

    ///
    /// \brief The EnumText class
    ///   Simple type of file:
    ///     This is string one        << id=1
    ///     And this string two       << id=2
    ///     String three              << id=3
    ///
    class EnumText final : public FileFormat
    {
        void doImport(Loader& loader) override;
        void doExport(Walker&) override {}

        /// @todo [future] can export too, but let’s import somehow
        Flags<Fcap> caps() const noexcept override { return Fcap::IMPORT; }
        std::unique_ptr<FileFormat> clone() override
            { return std::make_unique<EnumText>(*this); }
    };

}   // namespace tf
