#pragma once

// STL
#include <string>
#include <span>

// Libs
#include "u_Vector.h"

#include "TrDefines.h"

namespace tf {

    class Loader     // interface
    {
    public:
        virtual void addText(
                std::span<const std::u8string> ids,
                const std::u8string& original,
                const std::u8string& comment) = 0;
        /// Virtual dtor
        virtual ~Loader() = default;

        // Utils
        void addText(
                const std::u8string& id,
                const std::u8string& original,
                const std::u8string& comment)
            { addText(std::span<const std::u8string>(&id, 1), original, comment); }
    };

    struct TextInfo {
        int commonDepth = 0;
        int minusDepth = 0;
        SafeVector<std::u8string_view> ids;
        /// @warning  DO NOT use for dual-language data!
        ///           Use original() and translation() instead.
        std::u8string_view text;
        /// @warning  DO NOT use for single-language data!
        ///           Use text instead.
        std::u8string_view original, translation;

        int actualDepth() const { return ids.size() - 1; }
        bool isOk() const { return !eof(); }
        explicit operator bool() const { return isOk(); }
        bool eof() const { return ids.empty(); }
        int plusDepth() const { return actualDepth() - commonDepth; }
        bool groupChanged() const { return (commonDepth != actualDepth()) || (minusDepth != 0); }
        std::u8string joinGroupId(std::u8string_view sep) const;
        std::u8string_view textId() const { return ids.back(); }
    };

    class Walker    // interface
    {
    public:
        virtual ~Walker() = default;
        virtual const TextInfo& nextText() = 0;
    };

    class DummyProto final : public FormatProto
    {
    public:
        Flags<Fcap> caps() const noexcept override { return {}; }
        Flags<Usfg> workingSets() const noexcept override { return {}; }
        std::u8string_view locName() const override { return u8"None"; }
        constexpr std::string_view techName() const override { return "none"; }
        std::unique_ptr<FileFormat> make() const override;
        static const DummyProto INST;
    };

    class Dummy final : public FileFormat
    {
    public:
        std::unique_ptr<FileFormat> clone() override
            { return std::make_unique<Dummy>(*this); }

        const DummyProto& proto() const override { return DummyProto::INST; }
        void save(pugi::xml_node&) const override {}

        static Dummy INST;
    };

    ///
    /// \brief The EnumText class
    ///   Simple type of file:
    ///     This is string one        << id=1
    ///     And this string two       << id=2
    ///     String three              << id=3
    ///
//    class EnumText final : public FileFormat
//    {
//        LineBreakEscape lineBreakEscape;

//        void doImport(Loader& loader) override;
//        void doExport(Walker&) override {}

//        /// @todo [future] can export too, but let’s import somehow
//        Flags<Fcap> caps() const noexcept override { return Fcap::IMPORT; }
//        std::unique_ptr<FileFormat> clone() override
//            { return std::make_unique<EnumText>(*this); }

//        MobileInfo mobileInfo() const override;
//        void setMobileInfo(const MobileInfo& x) override;
//    };

    class IniProto : public FormatProto
    {
    public:
        Flags<Fcap> caps() const noexcept override { return Fcap::EXPORT; }
        Flags<Usfg> workingSets() const noexcept override
                { return Usfg::TEXT_FORMAT | Usfg::TEXT_ESCAPE | Usfg::MULTITIER; }
        std::unique_ptr<FileFormat> make() const override;
        std::u8string_view locName() const override { return u8"INI"; }
        constexpr std::string_view techName() const override { return "ini"; }

        static const IniProto INST;
    };

    class Ini final : public FileFormat
    {
    public:
        TextFormat textFormat;
        TextEscape textEscape;
        std::u8string separator = u8".";

        /// @todo [future] can import too, but let’s export somehow
        void doExport(Walker& walker, const std::filesystem::path& fname) override;

        std::unique_ptr<FileFormat> clone() override
            { return std::make_unique<Ini>(*this); }

        const IniProto& proto() const override { return IniProto::INST; }
        UnifiedSets unifiedSets() const override;
        void setUnifiedSets(const UnifiedSets& x) override;

        std::string bannedIdChars() const override;
        std::u8string bannedTextSubstring() const override
                { return textEscape.bannedSubstring(); }
        void save(pugi::xml_node&) const override;
    };

    enum {
        I_NONE,
        I_INI,
        I_N
    };
    extern const FormatProto* allProtos[I_N];

}   // namespace tf
