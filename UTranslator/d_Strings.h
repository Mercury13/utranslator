#pragma once

#include <string_view>

#define S8(x)  u8"" x

#define STR_UNTITLED       "(Untitled)"
#define STR_UNTRANSLATED   "(Not yet translated)"
#define STR_EMPTY_STRING   "(Empty string)"

#define EXT_ORIGINAL ".uorig"
#define WEXT_ORIGINAL W(EXT_ORIGINAL)
#define EXT_TRANSLATION ".utran"
#define WMASK(ext)  L"*" ext
#define W(s) (L"" s)

#define PAIR_ORIGINAL  L"Originals", WMASK(EXT_ORIGINAL)
#define FILTER_ORIGINAL { PAIR_ORIGINAL }

// We now translate originals only, and other translations are used as reference only!
#define FILTER_TRANSLATABLE FILTER_ORIGINAL
#define WEXT_TRANSLATABLE W(EXT_ORIGINAL)

#define PAIR_TRANSLATION L"Translations", WMASK(EXT_TRANSLATION)
#define FILTER_TRANSLATION { PAIR_TRANSLATION }
#define WEXT_TRANSLATION W(EXT_TRANSLATION)

#define PAIR_UTRANSL L"UTranslator files", \
    WMASK(EXT_ORIGINAL) L" " WMASK(EXT_TRANSLATION)
#define FILTER_UTRANSL { PAIR_UTRANSL }

constexpr const std::string_view langList[] = {
    "be", "cz", "cn", "de", "en", "es", "fr", "he", // Hebrew
    "hi",   // Hindi
    "it", "ja", "ru", "uk"
};

constexpr const char* STR_NEED_BILINGUAL_TRANSLATION =
        "This is possible for bilinguals/translations only.";

#define STR_SPACE_MAY_LEAD \
    "This may lead to problems in window composition, building of meaningful phrases."

#define STR_FIND_TRANSL \
    "This criterion works for translations only.\n"  \
        "It will find nothing in originals."
        /// @todo [bilingual] STR_FIND_TRANSL text for bilinguals
        //"In bilinguals it’s meaningful if portions of original are managed "
        //    "with external software (e.g. UI messages with form editor) "
        //    "and work as translation."

#define TAG_BR "<br>"
