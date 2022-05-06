#include "TrWrappers.h"

const tw::NoString tw::NoString::INST;


std::u8string_view tw::Flyweight::TranslStats::str() const
{
    snprintf(cache, std::size(cache), "%llu / %llu",
             static_cast<unsigned long long>(stats.nGood),
             static_cast<unsigned long long>(stats.nTexts));
    return str::toU8sv(cache);
}


tw::Fg tw::Flyweight::TranslStats::fg() const
{
    /// @todo [patch] Write smth else
    if (stats.nTexts == 0)
        return Fg::LIGHT;
    if (stats.nGood == 0)
        return Fg::UNTRANSLATED_CAT;
    if (stats.nGood == stats.nTexts)
        return Fg::OK;
    return Fg::ATTENTION;
}


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
    } else {
        auto& stats = x.stats(tr::StatsMode::CACHED, tr::CascadeDropCache::YES);
        trStats.stats = stats;
        return trStats;
    }
    return NoString::INST;
}
