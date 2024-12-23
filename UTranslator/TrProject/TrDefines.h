#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "u_TypedFlags.h"
#include "u_EcArray.h"
#include "u_OpenSaveStrings.h"
#include "u_Decoders.h"

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
    enum class WalkOrder : unsigned char {
        EXACT,      ///< Exact order: top down, s1 s2 s3 s4
        ECONOMY     ///< Economy order: never twice the same group, s1 s4 s2 s3
    };

    enum class WalkChannel : unsigned char { ORIGINAL, TRANSLATION };

    ///  Opaque handle, but may be used as interface
    class IconObject
    {
    public:
        virtual const char* iconName() const { return nullptr; }
        virtual ~IconObject() = default;
    };
    ///  Opaque handle for icons
    using HIcon = const IconObject*;
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
    using Super::reset;
    using Super::operator =;    

    CloningUptr(CloningUptr<T>&& x) noexcept
        : Super(std::move(x)) {}
    CloningUptr(std::unique_ptr<T>&& x) noexcept
        : Super(std::move(x)) {}
    CloningUptr<T>& operator = (CloningUptr<T>&& x) noexcept
        { Super::operator = (std::move(x)); return *this; }
    std::unique_ptr<T> upClone() const;
    CloningUptr<T> clone() const { return upClone(); }
};


template <UpCloneable T>
std::unique_ptr<T> CloningUptr<T>::upClone() const
{
    if (*this) {
        return (*this)->clone();
    } else {
        return {};
    }
}


namespace tr {

    ///  Implemented:
    ///   * Original: can fully edit
    ///   * Full translation: counts untranslated strings
    ///  Not implemented:
    ///   * Bilingual: has both original and full translation
    ///   * Patch translation: untranslated strings are not bugs;
    ///      someday so-called “triggers”: we react to some text in original
    ///   * Freestyle translation: original is not *.uorig but
    ///      some other project, for translation of e.g. game
    ///
    DEFINE_ENUM_TYPE_IN_NS(tr, PrjType, unsigned char,
        ORIGINAL, FULL_TRANSL)
    extern const ec::Array<const char*, PrjType> prjTypeNames;

    DEFINE_ENUM_TYPE_IN_NS(tr, PrefixSuffixMode, unsigned char,
        OFF, DFLT)

    struct PrjInfo {
        PrjType type = PrjType::ORIGINAL;
        struct Orig {
            std::string lang;
            std::filesystem::path absPath;
            /// @todo [bilingual] do smth with it, always false right now!
            bool isBilingual = false;
            bool operator == (const Orig& x) const = default;
        } orig;
        struct Ref {
            std::filesystem::path absPath;
            bool operator == (const Ref& x) const = default;
            void clear() { *this = Ref(); }
        } ref;
        struct Transl {
            std::string lang;            
            /// @warning  Pseudo-localization is applicable to full/bilingual only
            struct Pseudoloc {
                PrefixSuffixMode prefixSuffixMode = PrefixSuffixMode::OFF;

                bool operator == (const Pseudoloc& x) const = default;

                static const Pseudoloc OFF, DFLT;

                bool isDefault() const { return static_cast<bool>(prefixSuffixMode); }
                void setDefault(bool x) { prefixSuffixMode = static_cast<PrefixSuffixMode>(x); }
                bool isOn() const { return (*this != OFF); }
            } pseudoloc;
            bool operator == (const Transl& x) const = default;
            void clear() { *this = Transl(); }
        } transl;

        bool operator == (const PrjInfo& x) const = default;

        /// @return [+] can edit original, incl. adding files
        static bool canEditOriginal(PrjType type);
        bool canEditOriginal() const { return canEditOriginal(type); }

        /// @return [+] can have reference channel
        ///      We CANNOT have multi-storey translations EN→RU→UK.
        ///      But Ukrainian is close to Russian, and it’s better to translate
        ///      to Ukrainian from Russian → we may have reference channel.
        ///      Original is EN, translation is UK, reference is RU.
        static bool canHaveReference(PrjType type);
        bool canHaveReference() const { return canHaveReference(type); }

        /// @return [+] can add/remove files, edit orig. settings
        /// @warning  canEditOriginal → canAddFiles
        ///           (the converse is false for freestyle project)
        ///      @todo [freestyle, #12] we’ll have freestyle translation,
        ///                     it can edit files, but cannot edit original
        static bool canAddFiles(PrjType type);
        bool canAddFiles() const { return canAddFiles(type); }

        /// @return [+] Everything translation-related is available,
        ///             incl. two new channels (translation, translator’s comment),
        ///             translation settings…
        static bool isTranslation(PrjType type);
        bool isTranslation() const { return isTranslation(type); }

        /// @return [+] Can comment translations (false for original/bilingual
        static bool isTranslationCommentable(PrjType type) { return !canEditOriginal(type); }
        bool isTranslationCommentable() const { return isTranslationCommentable(type); }

        /// @return [+] is full translation, e.g. empty translation automatically needs attention.
        /// @warning  isFullTranslation → isTranslation
        /// @todo [patch, #23] what to do?
        bool isFullTranslation() const { return isTranslation(); }

        /// @return [+] original path matters
        /// @warning  hasOriginalPath → isTranslation
        ///           the converse is false for freestyle translation,
        ///           bilingual
        static bool hasOriginalPath(PrjType type);
        bool hasOriginalPath() const { return hasOriginalPath(type); }

        /// @return [+] Want pseudo-L10n in some manner
        ///         [-] Original is surely unchanged
        bool wantPseudoLoc() const
            { return isFullTranslation() && transl.pseudoloc.isOn(); }

        /// Turns settings to original’s
        void switchToOriginal(WalkChannel channel);

        /// Switches original and translation
        void switchOriginalAndTranslation(
                const std::filesystem::path& pathToNewOriginal);

        /// @return [+] actually has reference
        bool hasReference() const
        {
            // 1. Can have reference.
            // 2. Bilingual’s translation is a reference unless user specified another one.
            return canHaveReference() &&
                    (orig.isBilingual || !ref.absPath.empty());
        }
    };

//  Project types
//                           Original  Bilingual  Normal   Freestyle
//   (implemented)             YES        no       YES       no
//   canAddFiles               YES        YES      no        YES
//   canEditOriginal           YES        YES      no        no
//   canHaveReference          no         no       YES       no
//   isTranslation             no         YES      YES       YES
//   isTranslationCommentable  no         no       YES       YES    == !canEditOriginal
//   isFullTranslation         no       depends  depends   depends
//   hasOriginalPath           no         no       YES       no
//

    struct FileInfo {
        CloningUptr<tf::FileFormat> format;
        std::filesystem::path origPath, translPath;
        bool isIdless = false;
    };

}   // namespace tr
