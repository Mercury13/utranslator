#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "u_TypedFlags.h"

namespace pugi {
    class xml_node;
}

namespace tr {
    ///
    /// \brief The WalkOrder enum
    ///    s1
    ///    --group--s2
    ///             s3
    ///    s4
    ///
    enum class WalkOrder {
        EXACT,      ///< Exact order: top down, s1 s2 s3 s4
        ECONOMY     ///< Economy order: never twice the same group, s1 s4 s2 s3
    };
}

namespace tf {

    class Loader;
    class Walker;

    enum class Fcap {
        // FEATURES
        IMPORT = 1,
        EXPORT = 2,
        // EXPORT CONSTRAINTS, all need EXPORT flag
        /// [+] needs file and cannot export if it’s absent (e.g. Qt *.ui)
        /// [−] creates file from scratch (e.g. simple text/binary file, Transifex XLIFF)
        NEEDS_FILE = 4,
        NEEDS_ID = 8,           ///< [+] needs non-empty ID
        HAS_ORDER = 16,         ///< [+] Lines are made in specified order
    };
    DEFINE_ENUM_OPS(Fcap)

    enum class TextLineBreakStyle { LF, CRLF };
    constexpr auto TextLineBreakStyle_N = static_cast<int>(TextLineBreakStyle::CRLF) + 1;

    struct LineBreakStyleInfo {
        std::string_view techName;
        std::string_view locName;
        std::string_view eol;
    };
    extern const LineBreakStyleInfo textLineBreakStyleInfo[TextLineBreakStyle_N];

    enum class BinaryLineBreakStyle { CR, LF, CRLF };
    constexpr auto BinaryLineBreakStyle_N = static_cast<int>(BinaryLineBreakStyle::CRLF) + 1;

    extern const LineBreakStyleInfo binaryLineBreakStyleInfo[BinaryLineBreakStyle_N];

    enum class LineBreakEscapeMode {
        BANNED,         ///< Line-breaks banned
        C_CR,           ///< C mode: break = /r, / = //   (actually BACKslash here)
        C_LF,           ///< C mode: break = /n, / = //   (actually BACKslash here)
        SPECIFIED_TEXT  ///< Specified character that’s banned in text
    };
    constexpr auto LineBreakEscapeMode_N = static_cast<int>(LineBreakEscapeMode::SPECIFIED_TEXT) + 1;
    extern const char* const lineBreakEscapeModeNames[LineBreakEscapeMode_N];

    struct TextFormat {
        static constexpr auto DEFAULT_STYLE = TextLineBreakStyle::CRLF;
        bool writeBom = true;
        TextLineBreakStyle lineBreakStyle = DEFAULT_STYLE;
        std::string_view eol() const
            { return textLineBreakStyleInfo[static_cast<int>(lineBreakStyle)].eol; }
        std::string_view lineBreakTechName() const
            { return textLineBreakStyleInfo[static_cast<int>(lineBreakStyle)].techName; }
        static TextLineBreakStyle parseStyle(std::string_view name);
    };

    struct TechLoc {
        std::string_view techName;
        std::u8string_view locName;
    };

    enum class CSubformat {
        BARE,
        QUOTED,
        SLASH_SPACE
    };
    constexpr auto CSubformat_N = static_cast<int>(CSubformat::SLASH_SPACE) + 1;
    extern const TechLoc cSubformatInfo[CSubformat_N];

    ///  Principles of escaping line-breaks
    struct TextEscape {
        static constexpr std::u8string_view DEFAULT_LINE_BREAK_TEXT = u8"^";
        LineBreakEscapeMode mode = LineBreakEscapeMode::BANNED;
        std::u8string specifiedText { DEFAULT_LINE_BREAK_TEXT };
        CSubformat cSubformat;

        std::u8string bannedSubstring() const;
        std::u8string_view escapeSv(std::u8string_view x, std::u8string& cache) const;

        void setSpecifiedText(std::u8string_view x);
        /// @return [+] cSubformat means
        static bool isC(LineBreakEscapeMode mode);
        bool isC() const { return isC(mode); }
        /// @return [+] need to enquote C
        bool isQuoted() const { return isC() && cSubformat == CSubformat::QUOTED; }
    };

    enum class Usfg {
        TEXT_FORMAT = 1,
        TEXT_ESCAPE = 2,    ///< TEXT_ESCAPE implies TEXT_FORMAT
        MULTITIER = 4,      ///< has multitierSeparator
    };
    DEFINE_ENUM_OPS(Usfg)

    struct MultitierStyle {
        std::u8string separator = u8".";
    };

    ///
    ///  All possible settings of file
    ///  Very common settings that are often transfered from format to format
    ///  LineBreakStyle and EscapeInfo are probably mutually-exclusive:
    ///    first is when line breaks are actually emitted to text,
    ///    and second is when they are escaped somehow
    ///
    struct UnifiedSets {
        //LineBreakStyle binaryLineBreak = LineBreakStyle::LF;   unused right now
        TextFormat textFormat {};
        TextEscape textEscape {};
        MultitierStyle multitier {};
    };

    class FileFormat;

    struct ProtoFilter {
        Flags<Fcap> wantedCaps;
        bool allowEmpty;

        static const ProtoFilter ALL_EXPORTING_AND_NULL;
    };

    ///
    ///  Prototype for file format:
    ///  • Can create format objects
    ///  • Explains which of unified settings are available
    ///  • Ecplains importer’s capabilities
    ///
    class FormatProto   // interface
    {
    public:
        virtual ~FormatProto() = default;
        virtual Flags<Fcap> caps() const noexcept = 0;
        virtual Flags<Usfg> workingSets() const noexcept = 0;
        virtual std::unique_ptr<FileFormat> make() const = 0;
        /// @return  format’s localization name
        virtual std::u8string_view locName() const = 0;
        /// @return  format’s technical name
        constexpr virtual std::string_view techName() const noexcept = 0;
        virtual std::u8string_view locDescription() const = 0;
        virtual std::u8string_view locSoftware() const = 0;
        virtual std::u8string_view locIdType() const = 0;

        // Utils
        /// @return [+] format can import/export
        bool isWorking() const { return static_cast<bool>(caps()); }
        /// @return [+] format is dummy (= !isWorking, cannot import/export)
        bool isDummy() const { return !isWorking(); }
        bool isWithin(const ProtoFilter& filter) const;
    };

    ///
    /// \brief The FileInfo class
    ///   Common ancestor for file import/export
    ///
    class FileFormat    // interface
    {
    public:
        virtual ~FileFormat() = default;

        virtual void doImport(Loader&) {};
        virtual void doExport(Walker&, const std::filesystem::path&) {};

        virtual const FormatProto& proto() const = 0;

        virtual std::unique_ptr<FileFormat> clone() = 0;
        virtual UnifiedSets unifiedSets() const { return {}; }
        virtual void setUnifiedSets(const UnifiedSets&) {}

        /// @return characters banned in IDs
        virtual std::string bannedIdChars() const { return {}; }
        /// @return characters banned in texts
        virtual std::u8string bannedTextSubstring() const { return {}; }
        virtual tr::WalkOrder walkOrder() const = 0;

        virtual void save(pugi::xml_node&) const = 0;
        virtual void load(const pugi::xml_node&) = 0;

    protected:
        /// A common utility for unified saving.
        /// Most formats (enumerated text, INI) will not have any new code
        void unifiedSave(pugi::xml_node&) const;

        /// A common utility for unified loading
        /// Most formats (enumerated text, INI) will not have any new code
        void unifiedLoad(const pugi::xml_node&);
    };
}

template <class T>
concept UpCloneable = requires(T& t) {
    { t.clone() } -> std::convertible_to<std::unique_ptr<T>>;
};

template <UpCloneable T>
struct CloningUptr : public std::unique_ptr<T>
{
private:
    using Super = std::unique_ptr<T>;
public:
    using Super::Super;
    using Super::operator =;
    CloningUptr<T>& operator = (const CloningUptr<T>& x)
        { *this = x.clone(); }
};


namespace tr {

    enum class PrjType { ORIGINAL, FULL_TRANSL };
    constexpr int PrjType_N = static_cast<int>(PrjType::FULL_TRANSL) + 1;
    extern const char* const prjTypeNames[PrjType_N];

    struct PrjInfo {
        PrjType type = PrjType::ORIGINAL;
        struct Orig {
            std::string lang;
        } orig;
        struct Transl {
            std::string lang;
        } transl;
    };

    struct FileInfo {
        CloningUptr<tf::FileFormat> format;
        bool isIdless = false;
    };

}   // namespace tr
