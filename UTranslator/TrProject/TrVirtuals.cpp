// My header
#include "TrVirtuals.h"

// Libs
#include "mojibake.h"


constinit const tr::UpdateInfo tr::UpdateInfo::ZERO;


///// CanaryObject /////////////////////////////////////////////////////////////

tr::CanaryObject::CanaryObject() : canary(goodCanary()) {}


uint32_t tr::CanaryObject::goodCanary() const
{
    static constexpr size_t ALIGNMENT = alignof (uintptr_t);
    static constexpr size_t N_BITS = std::countr_zero(ALIGNMENT);
    static constexpr uint32_t SCRAMBLE = 0xC21330A5;
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) >> N_BITS) ^ SCRAMBLE;
}


void tr::CanaryObject::checkCanary() const
{
    if (canary != goodCanary())
        throw std::logic_error("[UiObject.checkCanary] Canary is dead, pointer to nowhere!");
}


tr::CanaryObject::~CanaryObject()
{
    canary = 0;
}


///// BigStats /////////////////////////////////////////////////////////////////


void tr::BigStats::LittleBigStats::add(const Translatable::Info& info)
{
    ++nStrings;
    nCpsOrig += info.nCpsOrig;
    nCpsTransl += info.nCpsTransl;
}


///// Translatable /////////////////////////////////////////////////////////////


tr::AttentionMode tr::Translatable::baseAttentionMode(const tr::PrjInfo& prjInfo) const
{
    // Translation → check for translation
    if (prjInfo.isTranslation()) {
        // Original changed → attention!
        if (knownOriginal)
            return tr::AttentionMode::AUTO_PROBLEM;
        // No translation: full → attention, patch → just BG
        if (!translation) {
            return prjInfo.isFullTranslation()
                    ? tr::AttentionMode::AUTO_PROBLEM
                    : tr::AttentionMode::BACKGROUND;
        }
        return tr::AttentionMode::CALM;
    } else {
        // Original: by definition, CALM!
        return tr::AttentionMode::CALM;
    }
}

tr::AttentionMode tr::Translatable::attentionMode(const tr::PrjInfo& prjInfo) const
{
    auto r = baseAttentionMode(prjInfo);
    // Force attention → attention!
    if (forceAttention)
        r = std::max(r, tr::AttentionMode::USER_ATTENTION);
    return r;
}


auto tr::Translatable::info(const tr::PrjInfo& prjInfo) const -> Info
{
    Info r;
    r.nCpsOrig = mojibake::countCps(original);
    if (translation && prjInfo.isTranslation())
        r.nCpsTransl = mojibake::countCps(*translation);
    return r;
}


///// UpdateInfo ///////////////////////////////////////////////////////////////


tr::UpdateInfo::ByState& tr::UpdateInfo::ByState::operator += (const ByState& x)
{
    nTranslated += x.nTranslated;
    nUntranslated += x.nUntranslated;
    return *this;
}


tr::UpdateInfo& tr::UpdateInfo::operator += (const UpdateInfo& x)
{
    nAdded += x.nAdded;
    deleted += x.deleted;
    changed += x.changed;
    return *this;
}


///// Stats ////////////////////////////////////////////////////////////////////


tr::Stats& tr::Stats::operator += (const Stats& x)
{
    nGroups += (x.nGroups + x.isGroup);
    text.nBackground += x.text.nBackground;
    text.nCalm += x.text.nCalm;
    text.nUserAttention += x.text.nUserAttention;
    text.nAutoProblem += x.text.nAutoProblem;
    text.nTranslated += x.text.nTranslated;
    text.nUntranslated += x.text.nUntranslated;
    return *this;
}


///// UiObject /////////////////////////////////////////////////////////////////


bool tr::UiObject::setOriginal(std::u8string_view x, tr::Modify wantModify)
{
    if (auto t = translatable()) {
        if (t->original != x) {
            t->original = x;
            stats(StatsMode::DIRECT, CascadeDropCache::YES);
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setIdless(bool x, tr::Modify wantModify)
{
    if (auto fi = ownFileInfo()) {
        if (fi->isIdless != x) {
            fi->isIdless = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
        }
    }
    return false;
}


bool tr::UiObject::setOrigPath(const std::filesystem::path& x, tr::Modify wantModify)
{
    if (auto fi = ownFileInfo()) {
        if (fi->origPath != x) {
            fi->origPath = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
        }
    }
    return false;
}


bool tr::UiObject::setTranslPath(const std::filesystem::path& x, tr::Modify wantModify)
{
    if (auto fi = ownFileInfo()) {
        if (fi->translPath != x) {
            fi->translPath = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::TRANSL);
            }
        }
    }
    return false;
}


bool tr::UiObject::setTranslation(
        std::optional<std::u8string_view> x, tr::Modify wantModify)
{
    if (auto t = translatable()) {
        if (t->translation != x) {
            t->translation = x;
            stats(StatsMode::DIRECT, CascadeDropCache::YES);
            if (wantModify != Modify::NO) {
                doModify(Mch::TRANSL);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::removeKnownOriginal(tr::Modify wantModify)
{
    if (auto t = translatable()) {
        if (t->knownOriginal) {
            t->knownOriginal.reset();
            if (wantModify != Modify::NO) {
                doModify(Mch::ORIG);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setAuthorsComment(std::u8string_view x, tr::Modify wantModify)
{
    if (auto c = comments()) {
        if (c->authors != x) {
            c->authors = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::COMMENT);
            }
            return true;
        }
    }
    return false;
}


bool tr::UiObject::setTranslatorsComment(std::u8string_view x, tr::Modify wantModify)
{
    if (auto c = comments()) {
        if (c->translators != x) {
            c->translators = x;
            if (wantModify != Modify::NO) {
                doModify(Mch::COMMENT);
            }
            return true;
        }
    }
    return false;
}


std::u8string tr::UiObject::makeTextId(const IdLib& idlib) const
{
    if (auto fi = inheritedFileInfo(); fi && !fi->isIdless) {
        return makeId(idlib.textPrefix, {});
    }
    return {};
}


bool tr::UiObject::moveUp(UiObject* aChild)
{
    if (!canMoveUp(aChild))
        return false;
    auto ind = aChild->cache.index;
    swapChildren(ind - 1, ind);
    aChild->doModify(Mch::META);
    return true;
}


bool tr::UiObject::moveDown(UiObject* aChild)
{
    if (!canMoveDown(aChild))
        return false;
    auto ind = aChild->cache.index;
    swapChildren(ind, ind + 1);
    aChild->doModify(Mch::META);
    return true;
}


void tr::UiObject::cascadeDropStats()
{
    cache.stats.reset();
    for (auto q = parent(); q; q = q->parent()) {
        q->cache.stats.reset();
    }
}


const tr::Stats& tr::UiObject::resetCacheIf(const Stats& r, CascadeDropCache cascade)
{
    if (cascade != CascadeDropCache::NO) {
        if (!cache.stats || *cache.stats != r) {
            cascadeDropStats();
        }
    }
    cache.stats = r;
    return *cache.stats;
}


tr::UpdateInfo tr::UiObject::addedInfo(CascadeDropCache cascade)
{
    tr::UpdateInfo r;
    auto& st = stats(StatsMode::CACHED, cascade);
    r.nAdded = st.text.nTotal();
    return r;
}


tr::UpdateInfo::ByState tr::UiObject::deletedInfo(CascadeDropCache cascade)
{
    tr::UpdateInfo::ByState r;
    auto& st = stats(StatsMode::DIRECT, cascade);
    r.nTranslated = st.text.nTranslated;
    r.nUntranslated = st.text.nUntranslated;
    return r;
}


void tr::UiObject::uiStealDataFrom(UiObject& x, UiObject* myParent)
{
    // Why myParent?
    // We do lots of trickery with parents and fix this afterwards
    if (myParent) {
        // Why so?
        // !myParent → we did not swap caches, just lists
        //   so cache of *this is GOOD, and cache of x is BAD
        cache.treeUi = std::move(x.cache.treeUi);
        // Father pointed to x? → now points to me
        if (myParent->cache.treeUi.currObject.lock() == x.selfUi()) {
            myParent->cache.treeUi.currObject = this->selfUi();
        }
    }
}
