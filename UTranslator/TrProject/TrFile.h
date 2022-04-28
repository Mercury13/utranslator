#pragma once

// STL
#include <string>
#include <span>

// Libs
#include "u_Vector.h"

#include "TrDefines.h"

namespace bom {
    constexpr std::string_view u8 = "\xEF\xBB\xBF";
}

namespace tf {

    ///
    /// @brief
    ///   To lessen O(n) search, we make Loader stateful.
    ///   E.g. we can go to any group using three functions,
    ///   then ad text.
    ///
    class Loader     // interface
    {
    public:
        /// Goes right to root group.
        virtual void goToRoot() = 0;

        /// Goes one group up;   [+] OK   [-] it’s root
        /// @throw  some strange problems
        virtual bool goUp() = 0;

        /// Goes to a subgroup. If it’s missing, creates it.
        virtual void goToGroupRel(std::u8string_view groupId) = 0;

        /// Adds a text. When overwriting and comment is empty →
        ///   SHOULD NOT TOUCH existing comment
        virtual void addText(
                std::u8string_view textId,
                std::u8string_view original,
                std::u8string_view comment) = 0;
        /// Virtual dtor
        virtual ~Loader() = default;

        // Utils
        /// Goes to absolute address.
        /// Equiv.to goToRoot, then several goToGroupRel.
        void goToGroupAbs(std::span<const std::u8string_view> groupIds);
        void goToGroupAbs(std::span<const std::u8string> groupIds);
    };

    struct TextInfo {
        int prevDepth = 0;
        int commonDepth = 0;
        /// SafeVector is OK, as std::vector never shrinks memory
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
        int minusDepth() const { return prevDepth - commonDepth; }
        int plusDepth() const { return actualDepth() - commonDepth; }

        /// @return  [+] group was changed compared to previous group
        bool groupChanged() const { return (commonDepth != actualDepth()) || (commonDepth != prevDepth); }

        std::u8string joinIdToDepth(std::u8string_view sep, size_t depth) const;
        std::u8string joinGroupId(std::u8string_view sep) const;
        std::u8string joinTextId(std::u8string_view sep) const;
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
        constexpr std::string_view techName() const noexcept override { return "none"; }
        std::unique_ptr<FileFormat> make() const override;
        static const DummyProto INST;
        std::u8string_view locDescription() const override;
        std::u8string_view locSoftware() const override { return u8"—"; }
        std::u8string_view locIdType() const override { return u8"—"; }
    };

    class Dummy final : public FileFormat
    {
    public:
        std::unique_ptr<FileFormat> clone() override
            { return std::make_unique<Dummy>(*this); }

        const DummyProto& proto() const override { return DummyProto::INST; }
        tr::WalkOrder walkOrder() const override { return tr::WalkOrder::ECONOMY; }
        void save(pugi::xml_node&) const override {}
        void load(const pugi::xml_node&) override {}
        filedlg::Filter fileFilter() const override { return {}; }

        static Dummy INST;
    };

    class IniProto : public FormatProto
    {
    public:
        Flags<Fcap> caps() const noexcept override
            { return Fcap::IMPORT | Fcap::EXPORT | Fcap::NEEDS_ID; }
        Flags<Usfg> workingSets() const noexcept override
            { return Usfg::TEXT_FORMAT | Usfg::TEXT_ESCAPE | Usfg::MULTITIER; }
        std::unique_ptr<FileFormat> make() const override;
        std::u8string_view locName() const override { return u8"INI"; }
        constexpr std::string_view techName() const noexcept override { return "ini"; }
        std::u8string_view locDescription() const override;
        std::u8string_view locSoftware() const override { return u8"Various"; }
        std::u8string_view locIdType() const override { return u8"String, 1-nested"; }

        static const IniProto INST;
    };

    class Ini final : public FileFormat
    {
    public:
        TextFormat textFormat;
        TextEscape textEscape;
        MultitierStyle multitier;

        /// @todo [future] can import too, but let’s export somehow
        void doExport(Walker& walker,
                      const std::filesystem::path&,
                      const std::filesystem::path& fname) override;

        std::unique_ptr<FileFormat> clone() override
            { return std::make_unique<Ini>(*this); }

        const IniProto& proto() const override { return IniProto::INST; }
        UnifiedSets unifiedSets() const override;
        void setUnifiedSets(const UnifiedSets& x) override;

        std::string bannedIdChars() const override;
        std::u8string bannedTextSubstring() const override
                { return textEscape.bannedSubstring(); }
        tr::WalkOrder walkOrder() const override { return tr::WalkOrder::ECONOMY; }
        void save(pugi::xml_node&) const override;
        void load(const pugi::xml_node&) override;
        filedlg::Filter fileFilter() const override;
    };

    enum {
        I_NONE,
        I_INI,
        I_N
    };
    extern const FormatProto* const allProtos[I_N];
    extern const FormatProto* const (&allWorkingProtos)[I_N - 1];

}   // namespace tf
