#pragma once

#include <filesystem>

namespace tr {

    class Project;

    // Extract original
    namespace eo {
        enum class Text : unsigned char { ORIG, TRANSL_ORIG };

        enum class Comment : unsigned char { AUTHOR, AUTHOR_TRANSL, TRANSL, TRANSL_AUTHOR };

        struct Sets {
            Text text = eo::Text::ORIG;
            Comment comment = eo::Comment::AUTHOR;
        };

        struct Sets2 {
            std::filesystem::path origPath;
            Comment comment = eo::Comment::AUTHOR;
        };
    }

    // Translate with
    namespace tw {
        enum class Priority : unsigned char { EXISTING, EXTERNAL };

        struct Sets {
            std::filesystem::path origPath;
            struct HoleSigns {
                bool originalIsTranslation = true;
            } holeSigns;
            Priority priority = Priority::EXISTING;
        };
    }

    void extractOriginal(Project& prj, const eo::Sets& sets);
    void switchOriginalAndTranslation(Project& prj, const eo::Sets2& sets);
    void resetKnownOriginal(Project& prj);
    void translateWithOriginal(Project& prj, const tw::Sets& sets);

}   // namespace tu
