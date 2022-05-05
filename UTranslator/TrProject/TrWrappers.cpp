#include "TrWrappers.h"

const tw::NoString tw::NoString::INST;


auto tw::Flyweight::getTransl(tr::UiObject& x) -> const TranslObj&
{
    if (auto tr = x.translatable()) {
        if (tr->translation) {
            if (tr->translation->empty()) {
                // Translated, empty string
                return dumb.set(l10n.emptyString, Fg::STATS);
            } else {
                // Translated, non-empty string
                return dumb.set(*tr->translation,
                        tr->knownOriginal ? Fg::ATTENTION : Fg::NORMAL);
            }
        } else {
            // Untranslated
            /// @todo [patch] Write smth like “Untouched”
            return dumb.set(l10n.untranslated, Fg::ATTENTION);
        }
    }
    return NoString::INST;
}
