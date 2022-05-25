#pragma once

#include "TrProject.h"
#include "mojibake.h"

namespace tr {

    enum class Bug {
        TR_EMPTY        = 1<<0,     ///< empty translation
        TR_ORIG_CHANGED = 1<<1,     ///< translation needs review
        COM_WARNING     = 1<<2,     ///< manual warning
        OR_SPACE_HEAD   = 1<<3,     ///< original: space in the beginning
        OR_SPACE_TAIL   = 1<<4,     ///< original: space in the end
        OR_LF_TAIL      = 1<<5,     ///< original: line break in the end
        TR_MULTILINE    = 1<<6,     ///< translation is multiline while original is not
        TR_SPACE_HEAD_ADD = 1<<7,   ///< translation: added heading space
        TR_SPACE_HEAD_DEL = 1<<8,   ///< translation: removed heading space
        TR_SPACE_TAIL_ADD = 1<<9,   ///< translation: added trailing space
        TR_SPACE_TAIL_DEL = 1<<10,  ///< translation: removed trailing space
        COM_WHITESPACE  = 1<<11,    ///< common: whitespace only
        COM_MOJIBAKE    = 1<<12,    ///< common: replacement character found
        TR_EMPTY_OK     = 1<<13,    ///< empty translation, and that’s OK

        ALL_SERIOUS = TR_EMPTY | TR_ORIG_CHANGED | COM_WARNING,
        ALL_INTERACTIVE = ALL_SERIOUS | TR_EMPTY_OK,
    };

    DEFINE_ENUM_OPS(Bug)

    enum class Mjf {
        ID = 1,
        ORIGINAL = 2,
        TRANSLATION = 4,
        COMMENT = 8
    };

    struct BugCache {
        std::u32string id {}, original {};
          ///< R/O, and as QString works quietly and with some mojibake → OK
        std::optional<std::u8string_view> knownOriginal {};
        std::u32string translation {};
        struct Comments {
            ///< R/O, and as QString works quietly and with some mojibake → OK
            std::u8string_view importers {};
            std::u32string editable {};
        } comm;

        std::weak_ptr<tr::UiObject> obj {};

        bool isProjectOriginal = false;
        bool hasTranslatable = false;
        bool hasComments = false;
        bool isProjectTranslation = false;
        bool isTranslationEmpty = false;
        /// Which editable fields have mojibake
        Flags<Mjf> moji {};

        bool canEditId() const { return isProjectOriginal; }
        bool canEditOriginal() const { return isProjectOriginal && hasTranslatable; }
        bool canEditTranslation() const { return isProjectTranslation && hasTranslatable; }

        void copyFrom(tr::UiObject& x);
        /// @warning  updates isTranslationEmpty flag
        void copyTo(tr::UiObject& x);
        void updateTransientFlags();

        /// Only for ID and comment
        Flags<Bug> smallBugsOf(
                std::u32string_view x,
                Mjf mojiFlag) const;
        Flags<Bug> bugsOf(
                std::u32string_view x,
                Mjf mojiFlag) const;
        Flags<Bug> bugs() const;
    };

}
