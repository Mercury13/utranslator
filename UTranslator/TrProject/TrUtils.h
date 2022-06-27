#pragma once

namespace tr {

    class Project;

    // Extract original
    namespace eo {
        enum class Text { ORIG, TRANSL, TRANSL_ORIG };

        enum class Comment { AUTHOR, AUTHOR_TRANSL, TRANSL, TRANSL_AUTHOR };

        struct Sets {
            Text text = eo::Text::ORIG;
            Comment comment = eo::Comment::AUTHOR;
        };
    }

    void extractOriginal(Project& prj, const eo::Sets& sets);

}   // namespace tu
