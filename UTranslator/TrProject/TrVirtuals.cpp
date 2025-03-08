// My header
#include "TrVirtuals.h"

// Libs
#include "mojibake.h"
#include "u_Strings.h"


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


void tr::UiObject::doModify(Mch ch)
{
    cache.mod.set(ch);
    if (auto p = vproject())
        p->modify();
}


void tr::UiObject::recache()
{
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        child(i)->cache.index = i;
    }
}


void tr::UiObject::recursiveRecache()
{
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        ch->cache.index = i;
        ch->recursiveRecache();
    }
}


void tr::UiObject::recursiveUnmodify()
{
    cache.mod.clear();
    auto nc = nChildren();
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        ch->recursiveUnmodify();
    }
}


std::u8string tr::UiObject::makeId(
        std::u8string_view prefix,
        std::u8string_view suffix) const
{
    auto nc = nChildren();
    size_t newIndex = 0;
    for (size_t i = 0; i < nc; ++i) {
        auto ch = child(i);
        auto sNumber = str::remainderSv(ch->idColumn(), prefix, suffix);
        size_t num = 0;
        auto chars = str::fromChars(sNumber, num);
        if (chars.ec == std::errc() && num >= newIndex) {
            newIndex = num + 1;
        }
    }
    char buf[30];
    auto sIndex = str::toCharsU8(buf, newIndex);

    std::u8string s;
    s.append(prefix);
    s.append(sIndex);
    s.append(suffix);
    return s;
}


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


void tr::UiObject::traverseTexts1(const EvMiniText& ev)
{
    auto q = [&ev](tr::UiObject&, tr::Translatable& tr) {
        ev(tr);
    };
    traverseTexts(q);
}


void tr::UiObject::traverseCTexts1(const EvMiniCText& ev) const
{
    auto q = [&ev](const tr::UiObject&, const tr::Translatable& tr) {
        ev(tr);
    };
    traverseCTexts(q);
}


void tr::UiObject::removeReferenceChannel()
{
    traverseTexts1([](tr::Translatable& tr) {
        tr.reference.reset();
    });
}


tr::BigStats tr::UiObject::bigStats() const
{
    BigStats r;
    if (auto prj = vproject()) {
        const auto& prjInfo = prj->prjInfo();
        traverseCTexts([&r, &prjInfo](const UiObject&, const Translatable& tr) {
            auto textInfo = tr.info(prjInfo);
            r.all.add(textInfo);
            if (prjInfo.isTranslation()) {
                // For all types of translations
                if (!tr.translation) {
                    r.untransl.add(textInfo);
                } else if (tr.attentionMode(prjInfo) > AttentionMode::CALM) {
                    r.dubious.add(textInfo);
                } else {
                    r.transl.add(textInfo);
                }
            } else {
                // For original
                if (tr.attentionMode(prjInfo) > AttentionMode::CALM) {
                    r.dubious.add(textInfo);
                } else {
                    r.untransl.add(textInfo);
                }
            }
        });
    }
    return r;
}


std::shared_ptr<tr::UiObject> tr::UiObject::extract(Modify wantModify)
{
    auto pnt = parent();
    // No parent?
    if (!pnt)
        return {};

    auto nc = pnt->nChildren();
    // Initial cache lookup
    if (static_cast<size_t>(cache.index) < nc
            && pnt->child(cache.index).get() == this) {
        return pnt->extractChild(cache.index, wantModify);
    }

    // Search by cache
    for (size_t i = 0; i < nc; ++i) {
        if (pnt->child(i).get() == this) {
            return pnt->extractChild(cache.index, wantModify);
        }
    }
    return {};
}


bool tr::UiObject::canMoveUp(const UiObject* aChild) const
{
    size_t index = aChild->cache.index;
    return (index > 0 && index < nChildren() && child(index).get() == aChild);
}


bool tr::UiObject::canMoveDown(const UiObject* aChild) const
{
    size_t index = aChild->cache.index;
    return (index + 1 < nChildren() && child(index).get() == aChild);
}


void tr::UiObject::swapChildren(size_t index1, size_t index2)
{
    doSwapChildren(index1, index2);
    child(index1)->cache.index = index1;
    child(index2)->cache.index = index2;
}


const tr::Stats& tr::UiObject::stats(StatsMode mode, CascadeDropCache cascade)
{
    if (cache.stats && mode == StatsMode::CACHED)
        return *cache.stats;

    if (mode == StatsMode::SEMICACHED)
        mode = StatsMode::CACHED;

    Stats r;
    r.isGroup = true;
    for (size_t i = 0; i < nChildren(); ++i) {
        auto ch = child(i);
        if (ch) {
            auto& chst = ch->stats(mode, CascadeDropCache::NO);
            r += chst;
        }
    }

    return resetCacheIf(r, cascade);
}
