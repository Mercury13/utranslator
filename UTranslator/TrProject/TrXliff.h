#pragma once

#include "TrProject.h"

namespace xlf {

    /// @todo [future] More bad ID policies
    enum class BadIdPolicy : unsigned char {
        KEEP, UNDERSCORE };

    enum class Priority : unsigned char {
        PROJECT, XLIFF };

    struct Sets {
        std::string idSeparator = ".";
        BadIdPolicy badIdPolicy = BadIdPolicy::UNDERSCORE;
        bool writeTranslation = true;  ///< [+] write translation if present
        bool writeCdata = true;        ///< [+] write CDATA if see &lt; or &gt;
        Priority priority = Priority::PROJECT;
    };

    void exportMe(
            const tr::Project& project,
            const std::filesystem::path& fname,
            Sets sets);

    void translate(
            tr::Project& project,
            const std::filesystem::path& fname,
            const Sets& sets);

}   // namespace xlf
