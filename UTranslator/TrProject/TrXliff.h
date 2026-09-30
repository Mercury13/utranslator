#pragma once

#include "TrProject.h"

namespace xlf {

    /// @todo [future] More bad ID policies
    enum class BadIdPolicy : unsigned char {
        KEEP, UNDERSCORE };

    enum class Priority : unsigned char {
        PROJECT, XLIFF };

    struct Sets {
        struct Id {
            std::string separator = ".";
            BadIdPolicy badPolicy = BadIdPolicy::UNDERSCORE;
        } id;
        struct WriteText {
            bool translation = true;  ///< [+] write translation if present
            bool cdata = true;        ///< [+] write CDATA if see &lt; or &gt;
        } writeText;
        struct Translate {
            Priority priority = Priority::PROJECT;
        } translate;
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
