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
    canEditOriginal = !prj || prj->info.canEditOriginal();

    // Translatable
    if (auto tr = x.translatable()) {
        hasTranslatable = true;
        hasTranslation = prj && prj->info.isTranslation();
        if (canEditOriginal && !mojibake::isValid(tr->original))
            moji |= Mjf::ORIGINAL;
        original = mojibake::toM<std::u32string>(tr->original);
        knownOriginal = tr->knownOriginal;  // optional permits this trick!
        if (tr->translation) {
            if (!mojibake::isValid(*tr->translation))
                moji |= Mjf::TRANSLATION;
            translation = mojibake::toM<std::u32string>(*tr->translation);
        }
    }

    if (auto com = x.comments()) {
        hasComments = true;
        auto& editComm = canEditOriginal ? com->authors : com->translators;
        if (!mojibake::isValid(editComm))
            moji |= Mjf::COMMENT;
        comm.editable = mojibake::toM<std::u32string>(editComm);
        comm.importers = com->importers;
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
    if (canEditOriginal) {
        r |= smallBugsOf(id, Mjf::ID);
        if (hasTranslatable) {
            r |= bugsOf(original, Mjf::ORIGINAL);
        }
    }

    // Translation
    if (hasTranslation) {
        if (translation) {
            r |= bugsOf(*translation, Mjf::TRANSLATION);
        } else {
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
