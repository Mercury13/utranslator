#pragma once

namespace tr {
    class Project;
}

namespace tu {

    // Extract original
    namespace eo {
        enum class Text { ORIG, TRANSL, TRANSL_ORIG };

        enum class Comment { AUTHOR, AUTHOR_TRANSL, TRANSL, TRANSL_AUTHOR };

        struct Sets {
            Text text;
            Comment comment;
        };
    }

}   // namespace tu
