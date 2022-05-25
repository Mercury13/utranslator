#include "TrBugs.h"


Flags<tr::Bug> tr::bugsOf(std::u32string_view x)
{
    Flags<tr::Bug> r;

    // Mojibake
    auto pMojibake = x.find(0xFFFD);
    if (pMojibake != std::u32string::npos) {
        r |= Bug::COM_MOJIBAKE;
        std::cout << "MOJIBAKE DETECTED!" << std::endl;
    }

    return r;
}


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

    /// @todo [bugs] what to do?
    bugsOf(original);
}
