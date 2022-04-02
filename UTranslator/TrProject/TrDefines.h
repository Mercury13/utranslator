#pragma once

#include <string>
#include <memory>

#include "u_TypedFlags.h"

namespace tf {

    class Loader;
    class Walker;

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
    class FileFormat
    {
    public:
        virtual void doImport(Loader& loader) = 0;
        virtual void doExport(Walker& walker) = 0;

        virtual Flags<Fcap> caps() const noexcept = 0;
        virtual ~FileFormat() = default;

        virtual std::unique_ptr<FileFormat> clone() = 0;
    };
}

template <class T>
concept UptrCloneable = requires(T& t) {
    { t.clone() } -> std::convertible_to<std::unique_ptr<T>>;
};

template <class T> requires UptrCloneable<T>
struct CloneableUptr : public std::unique_ptr<T>
{
private:
    using Super = std::unique_ptr<T>;
public:
    using Super::Super;
    using Super::operator =;
    CloneableUptr<T>& operator = (const CloneableUptr<T>& x)
        { *this = x.clone(); }
};

namespace tr {

    enum class PrjType { ORIGINAL, FULL_TRANSL };
    constexpr int PrjType_N = static_cast<int>(PrjType::FULL_TRANSL) + 1;
    extern const char* prjTypeNames[PrjType_N];

    enum class LineBreakMode { CR, LF, CRLF };
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
        CloneableUptr<tf::FileFormat> format;
        bool isIdless = false;
    };

}   // namespace tr
