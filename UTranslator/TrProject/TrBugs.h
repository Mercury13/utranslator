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

        ALL_SERIOUS = TR_EMPTY | TR_ORIG_CHANGED | COM_WARNING,
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
        std::optional<std::u32string> translation {};
        struct Comments {
            ///< R/O, and as QString works quietly and with some mojibake → OK
            std::u8string_view importers {};
            std::u32string editable {};
        } comm;

        std::weak_ptr<tr::UiObject> obj {};

        bool canEditOriginal = false;
        bool hasTranslatable = false;
        bool hasComments = false;
        bool hasTranslation = false;
        /// Which editable fields have mojibake
        Flags<Mjf> moji {};

        void copyFrom(tr::UiObject& x);
        std::u32string_view translationSv() const
            { return translation ? *translation : std::u32string_view{}; }

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
