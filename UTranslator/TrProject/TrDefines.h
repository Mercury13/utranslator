#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "u_TypedFlags.h"

namespace pugi {
    class xml_node;
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

    enum class LineBreakStyle { CR, LF, CRLF };
    constexpr auto LineBreakStyle_N = static_cast<int>(LineBreakStyle::CRLF) + 1;

    struct LineBreakStyleInfo {
        std::string_view techName;
        std::string_view eol;
    };
    extern LineBreakStyleInfo lineBreakStyleInfo[LineBreakStyle_N];

    enum class EscapeMode {
        NONE,           ///< Line-breaks banned
        C_CR,           ///< C mode: break = /r, / = //   (actually BACKslash here)
        C_LF,           ///< C mode: break = /n, / = //   (actually BACKslash here)
        SPECIFIED_TEXT  ///< Specified character that’s banned in text
    };

    struct TextFormat {
        bool writeBom = true;
        LineBreakStyle lineBreakStyle = LineBreakStyle::CRLF;
        std::string_view eol() const
            { return lineBreakStyleInfo[static_cast<int>(lineBreakStyle)].eol; }
    };

    ///  Principles of escaping line-breaks
    struct TextEscape {
        EscapeMode mode = EscapeMode::NONE;
        std::u8string specifiedText = u8"^";

        std::u8string bannedSubstring() const;
        std::u8string_view escape(std::u8string_view x, std::u8string& cache) const;
    };

    enum class Usfg {
        TEXT_FORMAT = 1,
        TEXT_ESCAPE = 2,    ///< TEXT_ESCAPE implies TEXT_FORMAT
        MULTITIER = 4,      ///< has multitierSeparator
    };
    DEFINE_ENUM_OPS(Usfg)

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
        char multitierSeparator = '.';
    };

    class FileFormat;

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
        constexpr virtual std::string_view techName() const = 0;

        // Utils
        /// @return [+] format can import/export
        bool isWorking() const { return caps().haveAny(Fcap::IMPORT | Fcap::EXPORT); }
        /// @return [+] format is dummy (= !isWorking, cannot import/export)
        bool isDummy() const { return !isWorking(); }
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

        virtual void save(pugi::xml_node&) const = 0;
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
    extern const char* prjTypeNames[PrjType_N];

    constexpr int LineBreakMode_N = static_cast<int>(PrjType::FULL_TRANSL) + 1;
    extern const char* lineBreakModeNames[PrjType_N];

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
