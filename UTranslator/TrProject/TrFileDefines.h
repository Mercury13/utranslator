#pragma once

#include <filesystem>
#include <span>
#include <optional>

#include "u_EnumSize.h"
#include "u_TypedFlags.h"
#include "u_EcArray.h"
#include "u_Decoders.h"
#include "u_OpenSaveStrings.h"

#include "TrDefines.h"

namespace pugi {
    class xml_node;
}

namespace tr {

    ///
    ///    s1
    ///    --group--s2
    ///             s3
    ///    s4
    ///
    enum class WalkOrder : unsigned char {
        EXACT,      ///< Exact order: top down, s1 s2 s3 s4
        ECONOMY     ///< Economy order: never twice the same group, s1 s4 s2 s3
    };

}

    namespace tf {

    class Loader;
    class Walker;

    enum class Fcap : unsigned char {
        // FEATURES
        IMPORT = 1,
        EXPORT = 2,
        XML = 4,
        // EXPORT CONSTRAINTS, all need EXPORT flag
        /// [+] needs file and cannot export if it’s absent (e.g. Qt *.ui)
        /// [−] creates file from scratch (e.g. simple text/binary file, Transifex XLIFF)
        NEEDS_FILE = 8,
        NEEDS_ID = 16,          ///< [+] needs non-empty ID
        HAS_ORDER = 32,         ///< [+] Lines are put in specified order
    };
    DEFINE_ENUM_OPS(Fcap)

    DEFINE_ENUM_TYPE_IN_NS(tf, TextLineBreakStyle, unsigned char,
                           LF, CRLF)

    struct LineBreakStyleInfo {
        std::string_view techName;
        std::string_view locName;
        std::string_view eol;
    };
    extern const ec::Array<LineBreakStyleInfo, TextLineBreakStyle> textLineBreakStyleInfo;

    //enum class BinaryLineBreakStyle { CR, LF, CRLF };

    //extern const ec::Array<LineBreakStyleInfo, BinaryLineBreakStyle> binaryLineBreakStyleInfo;

    struct TextFormat {
        static constexpr auto DEFAULT_STYLE = TextLineBreakStyle::CRLF;
        bool writeBom = true;
        TextLineBreakStyle lineBreakStyle = DEFAULT_STYLE;
        std::string_view eol() const
        { return textLineBreakStyleInfo[lineBreakStyle].eol; }
        std::string_view lineBreakTechName() const
        { return textLineBreakStyleInfo[lineBreakStyle].techName; }
        static TextLineBreakStyle parseStyle(std::string_view name);
    };

    struct TechLoc {
        std::string_view techName;
        std::u8string_view locName;
    };

    extern const ec::Array<std::string_view, escape::LineBreakMode> lineBreakEscapeModeNames;
    extern const ec::Array<TechLoc, escape::SpaceMode> spaceEscapeModeInfo;

    enum class Usfg : unsigned char {
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
        escape::Text textEscape {};
        MultitierStyle multitier {};
    };

    class FileFormat;

    struct ProtoFilter {
        Flags<Fcap> wantedCaps;
        bool allowEmpty;

        static const ProtoFilter ALL_EXPORTING_AND_NULL;
        static const ProtoFilter ALL_IMPORTING;
    };

    ///
    ///  Prototype for file format:
    ///  • Can create format objects
    ///  • Explains which of unified settings are available
    ///  • Ecplains importer’s capabilities
    ///
    class FormatProto : public tr::IconObject   // interface
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

    class FormatQueryObj  // interface
    {
    public:
        virtual std::optional<std::u8string> query(std::span<std::u8string_view> ids);
        virtual ~FormatQueryObj() = default;
    };

    ///
    /// \brief The FileInfo class
    ///   Common ancestor for file import/export
    ///
    class FileFormat    // interface
    {
    public:
        virtual ~FileFormat() = default;

        virtual void doImport(
            Loader&, const std::filesystem::path&) {};
        virtual void doExport(
            Walker&,
            [[maybe_unused]] const std::filesystem::path& fnExisting,
            [[maybe_unused]] const std::filesystem::path& fnExported) {};
        ///  This object works around banned ID chars, requiring the host
        ///    to query through a sequence of IDs.
        ///  For "Translate with → Exported directory" only.
        ///  @warning  Useless when cannot save
        ///  @warning  The object must be self-sufficient, i.e. do not rely
        ///    on the existence of FileFormat caller
        std::unique_ptr<FormatQueryObj> doImportAsQuery(
            const std::filesystem::path&) { return {}; }

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
        virtual filedlg::Filter fileFilter() const = 0;

    protected:
        /// A common utility for unified saving.
        /// Most formats (enumerated text, INI) will not have any new code
        void unifiedSave(pugi::xml_node&) const;

        /// A common utility for unified loading
        /// Most formats (enumerated text, INI) will not have any new code
        void unifiedLoad(const pugi::xml_node&);
    };

    enum class LoadTo : unsigned char { SELECTED, ROOT };
    enum class Existing : unsigned char { KEEP, OVERWRITE };
    struct LoadTextsSettings {
        LoadTo loadTo;
        Existing existing;
    };

    enum class TextOwner : unsigned char { EDITOR, ME };

    /// Mode for stealing original data
    /// @warning same order as TextOwner
    ///     *this (current object) is from EXTERNAL SOFTWARE, x is HAND-EDITED
    ///     so EDITOR is external software → then KEEP
    ///     EDITOR is my program → then STEAL
    ///
    enum class StealOrig : unsigned char {
        KEEP,           ///< Keep original data
        STEAL,          ///< Steal original data
        KEEP_WARN       ///< Same as KEEP + use knownOrig
    };

    constexpr int TextOwner_N = static_cast<int>(TextOwner::ME) + 1;
    extern const char* const textOwnerNames[TextOwner_N];

    struct SyncInfo {
        TextOwner textOwner = TextOwner::ME;
    };

}   // namespace tf

namespace tr {
    struct FileInfo {
        CloningUptr<tf::FileFormat> format;
        std::filesystem::path origPath, translPath;
        bool isIdless = false;
    };
}