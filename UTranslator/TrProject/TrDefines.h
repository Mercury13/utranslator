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
    };


}

namespace tr {

    enum class PrjType { ORIGINAL, FULL_TRANSL };
    constexpr int PrjType_N = static_cast<int>(PrjType::FULL_TRANSL) + 1;
    extern const char* prjTypeNames[PrjType_N];

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
        bool isIdless = false;
        std::unique_ptr<tf::FileFormat> format;
    };

}   // namespace tr
