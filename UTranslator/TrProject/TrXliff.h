#pragma once

#include <string>
#include <filesystem>

namespace tr {

    class Project;

    /// @todo [future] More bad ID policies
    enum class BadIdPolicy : unsigned char {
        KEEP, UNDERSCORE };

    enum class Priority : unsigned char {
        PROJECT, XLIFF };

    struct XliffSets {
        std::string idSeparator = ".";
        BadIdPolicy badIdPolicy = BadIdPolicy::UNDERSCORE;
        bool writeTranslation = true;  ///< [+] write translation if present
        bool writeCdata = true;        ///< [+] write CDATA if see &lt; or &gt;
        Priority priority = Priority::PROJECT;
    };

    void exportToXliff(
            const tr::Project& project,
            const std::filesystem::path& fname,
            XliffSets& sets);

}   // namespace tr