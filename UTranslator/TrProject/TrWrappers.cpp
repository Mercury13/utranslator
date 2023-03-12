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
    if (stats.text.nTotalAttention() > 0) {
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


namespace {

    constinit tw::Fg attentionToFg[tr::AttentionMode_N] = {
        tw::Fg::LIGHT, tw::Fg::NORMAL, tw::Fg::ATTENTION
    };

}   // anon namespace

auto tw::Flyweight::getTransl(tr::UiObject& x) -> const TranslObj&
{
    if (auto tr = x.translatable()) {
        auto attention = tr->attentionMode(x.project()->info);
        auto foregnd = attentionToFg[static_cast<int>(attention)];
        if (tr->translation) {
            if (tr->translation->empty()) {
                // Translated, empty string
                // turn calm to smth lighter
                return dumb.set(l10n.emptyString,
                    (attention == tr::AttentionMode::CALM) ? Fg::STATS : foregnd);
            } else {
                // Translated, non-empty string
                return dumb.set(*tr->translation, foregnd);
            }
        } else {
            // Untranslated
            /// @todo [patch, #23] Write smth like “Untouched”
            ///     (use attention mode)
            return dumb.set(l10n.untranslated, foregnd);
        }
    } else {
        auto& stats = x.stats(tr::StatsMode::CACHED, tr::CascadeDropCache::YES);
        trStats.stats = stats;
        return trStats;
    }
    return NoString::INST;
}
