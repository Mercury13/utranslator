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
    obj = x.selfUi();
    // ID
    id = obj->idColumn();

    // Translatable
    if (auto tr = x.translatable()) {
        original = unicode::convert<std::u8string, std::u32string>(tr->original);
        knownOriginal = tr->knownOriginal;  // optional permits this trick!
        if (tr->translation) {
            translation = unicode::convert<std::u8string, std::u32string>(*tr->translation);
        } else {
            translation.reset();
        }
    } else {
        original.clear();
        knownOriginal.reset();
        translation.reset();
    }

    auto prj = x.project();
    if (auto com = x.comments()) {
        if (!prj || prj->info.canEditOriginal()) {
            comm.editable = com->authors;
        } else {
            comm.editable = com->importers;
        }
        comm.importers = com->importers;
    } else {
        comm.importers = std::u8string_view{};
        comm.editable.clear();
    }

    /// @todo [bugs] what to do?
    bugsOf(original);
}
