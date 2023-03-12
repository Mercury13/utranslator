#pragma once

#include <filesystem>

namespace tr {

    class Project;

    // Extract original
    namespace eo {
        enum class Text { ORIG, TRANSL_ORIG };

        enum class Comment { AUTHOR, AUTHOR_TRANSL, TRANSL, TRANSL_AUTHOR };

        struct Sets {
            Text text = eo::Text::ORIG;
            Comment comment = eo::Comment::AUTHOR;
        };

        struct Sets2 {
            std::filesystem::path origPath;
            Comment comment = eo::Comment::AUTHOR;
        };
    }

    void extractOriginal(Project& prj, const eo::Sets& sets);
    void switchOriginalAndTranslation(Project& prj, const eo::Sets2& sets);

}   // namespace tu
