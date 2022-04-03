#pragma once

#include <string>
#include <memory>

#include "u_TypedFlags.h"

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

    enum class EscapeMode {
        NONE,           ///< Line-breaks banned
        C_CR,           ///< C mode: break = /r, / = //   (actually BACKslash here)
        C_LF,           ///< C mode: break = /n, / = //   (actually BACKslash here)
        SPECIFIED_CHAR  ///< Specified character that’s banned in text
    };

    ///  Principles of escaping line-breaks
    struct EscapeInfo {
        EscapeMode mode = EscapeMode::NONE;
        char specifiedChar = '^';
    };

    ///
    ///  Very common settings that are often transfered from format to format
    ///  LineBreakStyle and EscapeInfo are probably mutually-exclusive.
    ///
    struct MobileInfo {
        LineBreakStyle lineBreakMode = LineBreakStyle::LF;
        EscapeInfo escapeInfo {};
        char multitierSeparator = '.';
    };

    ///
    /// \brief The FileInfo class
    ///   Common ancestor for file import/export
    ///
    class FileFormat
    {
    public:
        virtual void doImport(Loader& loader) = 0;
        virtual void doExport(Walker& walker) = 0;

        virtual Flags<Fcap> caps() const noexcept = 0;
        virtual ~FileFormat() = default;

        virtual std::unique_ptr<FileFormat> clone() = 0;
        virtual MobileInfo mobileInfo() const { return {}; }
        virtual void setMobileInfo(const MobileInfo&) {}
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
