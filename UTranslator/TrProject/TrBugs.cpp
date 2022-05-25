#include "TrBugs.h"


void tr::BugCache::copyFrom(tr::UiObject& x)
{
    *this = BugCache{};
    obj = x.selfUi();

    // ID
    auto idc = x.idColumn();

    if (!mojibake::isValid(idc))
        moji |= Mjf::ID;
    id = mojibake::toM<std::u32string>(idc);

    auto prj = x.project();
    isProjectOriginal = !prj || prj->info.canEditOriginal();

    // Translatable
    if (auto tr = x.translatable()) {
        hasTranslatable = true;
        isProjectTranslation = prj && prj->info.isTranslation();
        if (isProjectOriginal && !mojibake::isValid(tr->original))
            moji |= Mjf::ORIGINAL;
        original = mojibake::toM<std::u32string>(tr->original);
        knownOriginal = tr->knownOriginal;  // optional permits this trick!
        if (tr->translation) {
            if (!mojibake::isValid(*tr->translation))
                moji |= Mjf::TRANSLATION;
            translation = mojibake::toM<std::u32string>(*tr->translation);
            isTranslationEmpty = tr->translation->empty();
        }
    }

    if (auto com = x.comments()) {
        hasComments = true;
        auto& editComm = isProjectOriginal ? com->authors : com->translators;
        if (!mojibake::isValid(editComm))
            moji |= Mjf::COMMENT;
        comm.editable = mojibake::toM<std::u32string>(editComm);
        comm.importers = com->importers;
    }
}


void tr::BugCache::copyTo(
        tr::UiObject& x, const BugCache& oldCache, Flags<tr::Bug> bugsToRemove)
{
    // ID
    if (hasId() && id != oldCache.id)
        x.setId(mojibake::toM<std::u8string>(id), tr::Modify::YES);

    // Original
    if (canEditOriginal() && original != oldCache.original)
        x.setOriginal(mojibake::toM<std::u8string>(original), tr::Modify::YES);

    // Translation
    bool removeEmpty = translation.empty() || bugsToRemove.have(Bug::TR_EMPTY);
    if (canEditTranslation()
            && ((translation != oldCache.translation) || removeEmpty)) {
        std::optional<std::u8string> tmp;
        if (translation.empty()) {
            if (isTranslationEmpty || removeEmpty)   // was empty, or
                tmp.emplace();
        } else {
            tmp = mojibake::toM<std::u8string>(translation);
        }
        x.setTranslation(tmp, tr::Modify::YES);

        // Update that flag!!
        isTranslationEmpty = tmp && tmp->empty();
    }

    // Comment
    if (hasComments && comm.editable != oldCache.comm.editable) {
        auto tmp = mojibake::toM<std::u8string>(comm.editable);
        if (isProjectOriginal) {
            x.setAuthorsComment(tmp, tr::Modify::YES);
        } else {
            x.setTranslatorsComment(tmp, tr::Modify::YES);
        }
    }
}


Flags<tr::Bug> tr::BugCache::smallBugsOf(
        std::u32string_view x,
        Mjf mojiFlag) const
{
    Flags<tr::Bug> r;
    if (moji.have(mojiFlag)) {
        if (x.find(mojibake::MOJIBAKE) != std::u32string_view::npos) {
            r |= tr::Bug::COM_MOJIBAKE;
        }
    }
    return r;
}


Flags<tr::Bug> tr::BugCache::bugsOf(
        std::u32string_view x,
        Mjf mojiFlag) const
{
    Flags<tr::Bug> r = smallBugsOf(x, mojiFlag);
    return r;
}


Flags<tr::Bug> tr::BugCache::bugs() const
{
    Flags<tr::Bug> r;

    // ID / original
    if (isProjectOriginal) {
        r |= smallBugsOf(id, Mjf::ID);
        if (hasTranslatable) {
            r |= bugsOf(original, Mjf::ORIGINAL);
        }
    }

    // Translation
    if (isProjectTranslation) {
        r |= bugsOf(translation, Mjf::TRANSLATION);
        if (translation.empty() && !isTranslationEmpty) {
            r |= Bug::TR_EMPTY;
        }
        if (knownOriginal)
            r |= Bug::TR_ORIG_CHANGED;
    }

    // Comment
    if (hasComments) {
        r |= bugsOf(comm.editable, Mjf::COMMENT);
    }

    return r;
}
