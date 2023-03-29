#pragma once

#include "TrProject.h"

namespace tr {

    enum class Bug {
        TR_EMPTY        = 1<<0,     ///< empty translation
        TR_ORIG_CHANGED = 1<<1,     ///< translation needs review
        COM_WARNING     = 1<<2,     ///< TODO manual warning
        OR_SPACE_HEAD   = 1<<3,     ///< TODO original: space in the beginning
        OR_SPACE_TAIL   = 1<<4,     ///< TODO original: space in the end
        OR_LF_TAIL      = 1<<5,     ///< TODO original: line break in the end
        TR_MULTILINE    = 1<<6,     ///< translation is multiline while original is not
        TR_SPACE_HEAD_ADD = 1<<7,   ///< translation: added heading space
        TR_SPACE_HEAD_DEL = 1<<8,   ///< translation: removed heading space
        TR_SPACE_TAIL_ADD = 1<<9,   ///< translation: added trailing space
        TR_SPACE_TAIL_DEL = 1<<10,  ///< translation: removed trailing space
        COM_INVISIBLE   = 1<<11,    ///< common: invisible chars only
        COM_MOJIBAKE    = 1<<12,    ///< common: replacement character found
        OR_EMPTY       = 1<<13,     ///< original: empty string

        ALL_SERIOUS = TR_EMPTY | TR_ORIG_CHANGED | COM_WARNING,
    };

    DEFINE_ENUM_OPS(Bug)

    enum class Mjf {
        ID = 1,
        ORIGINAL = 2,
        TRANSLATION = 4,
        COMMENT = 8
    };

    struct BugCache
    {
        std::u32string id {}, original {};
          ///< R/O, and as QString works quietly and with some mojibake → OK
        std::optional<std::u32string> knownOriginal {}, reference {};
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
        bool hasId() const { return hasComments; }

        void copyFrom(tr::UiObject& x);
        /// @param [in] oldCache  condition when we copied FROM
        /// @warning  updates isTranslationEmpty flag
        void copyTo(tr::UiObject& x, const BugCache& oldCache, Flags<tr::Bug> bugsToRemove);
        void updateTransientFlags();

        /// Only for ID and comment
        Flags<Bug> smallBugsOf(
                std::u32string_view x,
                Mjf mojiFlag) const;
        Flags<Bug> bugsOf(
                std::u32string_view x,
                Mjf mojiFlag) const;
        Flags<Bug> origBugsOf(std::u32string_view x) const;
        Flags<Bug> doubleBugsOf(std::u32string_view ori, std::u32string_view tra) const;
        Flags<Bug> bugs() const;
    private:
        using This = BugCache;
    };

}
