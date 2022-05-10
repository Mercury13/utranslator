#include "TrWrappers.h"

const tw::NoString tw::NoString::INST;


std::u8string_view tw::Flyweight::TranslStats::str() const
{
    snprintf(cache, std::size(cache), "%llu / %llu",
             static_cast<unsigned long long>(stats.text.nCalm),
             static_cast<unsigned long long>(stats.text.nTotal()));
    return str::toU8sv(cache);
}


tw::Fg tw::Flyweight::TranslStats::fg() const
{
    if (stats.text.nAttention > 0) {
        if (stats.text.nCalm == 0 && stats.text.nBackground == 0) {
            // ALL need attention
            return Fg::UNTRANSLATED_CAT;
        } else {
            return Fg::ATTENTION;
        }
    }
    if (stats.text.nCalm > 0)
        return Fg::OK;
    return Fg::LIGHT;
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
            /// @todo [patch, #23] Write smth like “Untouched”
            return dumb.set(l10n.untranslated, Fg::ATTENTION);
        }
    } else {
        auto& stats = x.stats(tr::StatsMode::CACHED, tr::CascadeDropCache::YES);
        trStats.stats = stats;
        return trStats;
    }
    return NoString::INST;
}
