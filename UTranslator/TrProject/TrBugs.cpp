// My header
#include "TrBugs.h"

// Utils
#include "mojibake.h"

void tr::BugCache::copyFrom(tr::UiObject& x)
{
    *this = This{};
    obj = x.selfUi();

    // ID
    auto idc = x.idColumn();

    if (!mojibake::isValid(idc))
        moji |= Mjf::ID;
    id = mojibake::toM<std::u32string>(idc);

    auto prj = x.vproject();
    isProjectOriginal = !prj || prj->prjInfo().canEditOriginal();

    // Translatable
    if (auto tr = x.translatable()) {
        hasTranslatable = true;
        isProjectTranslation = prj && prj->prjInfo().isTranslation();
        if (isProjectOriginal && !mojibake::isValid(tr->original))
            moji |= Mjf::ORIGINAL;
        original = mojibake::toM<std::u32string>(tr->original);
        if (tr->knownOriginal) {
            knownOriginal = mojibake::toM<std::u32string>(*tr->knownOriginal);
        }
        if (tr->reference) {
            reference = mojibake::toM<std::u32string>(*tr->reference);
        }
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
    bool removeEmpty = translation.empty() && bugsToRemove.have(Bug::TR_EMPTY);
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

    if (bugsToRemove.have(tr::Bug::TR_ORIG_CHANGED)) {
        if (x.removeKnownOriginal(tr::Modify::YES))
            knownOriginal.reset();
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


namespace {

    bool isControl(char32_t c)
    {
        return (c <= 31
                || (c >= 0x7F && c <= 0x9F));
    }

    bool isWhitespace(char32_t c)
    {
        switch (c) {
        case 0x0020:    // space
        case 0x00A0:    // nbsp
            // ogham space mark is NOT whitespace here
        case 0x2000:    // En Quad
        case 0x2001:    // Em Quad
        case 0x2002:    // En Space
        case 0x2003:    // Em Space
        case 0x2004:    // Three-Per-Em Space
        case 0x2005:    // Four-Per-Em Space
        case 0x2006:    // Six-Per-Em Space
        case 0x2007:    // Figure Space
        case 0x2008:    // Punctuation Space
        case 0x2009:    // Thin Space
        case 0x200A:    // Hair Space
        case 0x202F:    // Narrow No-Break Space (NNBSP)
        case 0x205F:    // Medium Mathematical Space (MMSP)
        case 0x3000:    // Ideographic Space
            return true;
        default:
            return false;
        }
    }

    bool isInvisible(char32_t c) {
        return isControl(c) || isWhitespace(c);
    }

    bool startsWithInvisible(std::u32string_view s)
    {
        /// @todo [future] With ignorable:
        ///      skip all those ignorable, the 1st should be control/space
        return !s.empty() && isInvisible(s[0]);
    }

    bool endsWithInvisible(std::u32string_view s)
    {
        /// @todo [future] With ignorable:
        ///      skip all those ignorable, the 1st should be control/space
        return !s.empty() && isInvisible(s.back());
    }

    bool isStrInvisible(std::u32string_view x)
    {
        if (x.empty())
            return false;
        for (auto c : x) {
            if (!isInvisible(c)) {
                return false;
            }
        }
        return true;
    }

    bool isMultiline(std::u32string_view x)
        { return (x.find('\n') != std::u32string_view::npos); }

}   // anon namespace


Flags<tr::Bug> tr::BugCache::bugsOf(std::u32string_view x, Mjf mojiFlag) const
{
    Flags<tr::Bug> r = smallBugsOf(x, mojiFlag);
    if (isStrInvisible(x))
        r |= Bug::COM_INVISIBLE;
    return r;
}


Flags<tr::Bug> tr::BugCache::origBugsOf(std::u32string_view x) const
{
    auto r = bugsOf(x, Mjf::ORIGINAL);
    if (x.empty())
        r |= Bug::OR_EMPTY;
    return r;
}


Flags<tr::Bug> tr::BugCache::doubleBugsOf(
        std::u32string_view ori, std::u32string_view tra) const
{
    auto r = bugsOf(tra, Mjf::TRANSLATION);
    if (!isMultiline(ori) && isMultiline(tra))
        r |= Bug::TR_MULTILINE;
    if (!ori.empty() && !tra.empty()
            && !isStrInvisible(ori) && !isStrInvisible(tra)) {
        bool oriBeg = startsWithInvisible(ori);
        bool traBeg = startsWithInvisible(tra);
        if (oriBeg != traBeg) {
            r |= (traBeg ? Bug::TR_SPACE_HEAD_ADD : Bug::TR_SPACE_HEAD_DEL);
        }
        bool oriEnd = endsWithInvisible(ori);
        bool traEnd = endsWithInvisible(tra);
        if (oriEnd != traEnd) {
            r |= (traEnd ? Bug::TR_SPACE_TAIL_ADD : Bug::TR_SPACE_TAIL_DEL);
        }
    }
    return r;
}


Flags<tr::Bug> tr::BugCache::bugs() const
{
    Flags<tr::Bug> r;

    // ID / original
    if (isProjectOriginal) {
        r |= smallBugsOf(id, Mjf::ID);
        if (hasTranslatable) {
            r |= origBugsOf(original);
        }
    }

    // Translation
    if (isProjectTranslation) {
        r |= bugsOf(translation, Mjf::TRANSLATION);
        r |= doubleBugsOf(original, translation);
        if (translation.empty() && !isTranslationEmpty) {
            r |= Bug::TR_EMPTY;
        }
        if (knownOriginal)
            r |= Bug::TR_ORIG_CHANGED;
    }

    // Comment
    if (hasComments) {
        r |= smallBugsOf(comm.editable, Mjf::COMMENT);
    }

    // Object itself
    { auto lobj = obj.lock();
        if (auto tr = lobj->translatable()) {
            if (tr->forceAttention) {
                r |= Bug::COM_ATTENTION;
            }
        }
    }

    return r;
}
