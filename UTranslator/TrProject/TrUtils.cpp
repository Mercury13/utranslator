// My header
#include "TrUtils.h"

// Project
#include "TrProject.h"


namespace {

    class ProcessTr {   // interface
    public:
        virtual void act(tr::Translatable& x) const = 0;
        virtual ~ProcessTr() = default;
    };

    class ProcessCm {   // interface
    public:
        virtual void act(tr::Comments& x) const = 0;
        virtual ~ProcessCm() = default;
    };

    class TrOrig final : public ProcessTr {
    public:
        void act(tr::Translatable&) const override {};
    };

    class TrTranslOrig final : public ProcessTr {
    public:
        void act(tr::Translatable& x) const override {
            if (x.translation)
                x.original = *x.translation;
        };
    };

    const TrOrig TR_ORIG;
    const TrTranslOrig TR_TRANSL_ORIG;

    class CmAuthor final : public ProcessCm {
    public:
        void act(tr::Comments&) const override {};
    };

    class CmAuthorTransl final : public ProcessCm {
    public:
        void act(tr::Comments& x) const override
        {
            if (x.authors.empty())
                x.authors = std::move(x.translators);
        };
    };

    class CmTransl final : public ProcessCm {
    public:
        void act(tr::Comments& x) const override
        {
            std::swap(x.authors, x.translators);
        };
    };

    class CmTranslAuthor final : public ProcessCm {
    public:
        void act(tr::Comments& x) const override
        {
            if (!x.translators.empty())
                std::swap(x.authors, x.translators);
        };
    };

    constinit const CmAuthor CM_AUTHOR;
    constinit const CmAuthorTransl CM_AUTHOR_TRANSL;
    constinit const CmTransl CM_TRANSL;
    constinit const CmAuthorTransl CM_TRANSL_AUTHOR;

    constinit const ec::Array<const ProcessTr*, tr::eo::Text> extractOriginalTr {
        &TR_ORIG, &TR_TRANSL_ORIG
    };

    constinit const ec::Array<tr::WalkChannel, tr::eo::Text> extractOriginalChannel {
        tr::WalkChannel::ORIGINAL, tr::WalkChannel::TRANSLATION
    };

    constinit const ec::Array<const ProcessCm*, tr::eo::Comment> extractOriginalCm {
        &CM_AUTHOR, &CM_AUTHOR_TRANSL, &CM_TRANSL, &CM_TRANSL_AUTHOR
    };

    class ExtractOriginal final : public tr::TraverseListener
    {
    public:
        ExtractOriginal(const tr::eo::Sets& aSets);
        void onText(const std::shared_ptr<tr::Text>&) override;
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
    private:
        const ProcessTr& processTr;
        const ProcessCm& processCm;
    };

    ExtractOriginal::ExtractOriginal(const tr::eo::Sets& aSets)
        : processTr(*extractOriginalTr[aSets.text]),
          processCm(*extractOriginalCm[aSets.comment]) {}

    void ExtractOriginal::onText(const std::shared_ptr<tr::Text>& text)
    {
        processTr.act(text->tr);
        processCm.act(text->comm);
    }

    void ExtractOriginal::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& group)
    {
        processCm.act(group->comm);
    }

}   // anon namespace


void tr::extractOriginal(Project& prj, const eo::Sets& sets)
{
    ExtractOriginal listener(sets);
    prj.traverse(listener, WalkOrder::ECONOMY, EnterMe::NO);
    prj.removeTranslChannel();
    prj.info.switchToOriginal(extractOriginalChannel[sets.text]);
    prj.fname.clear();
    prj.modify();
}


namespace {

    class SwitchOriginalAndTranslation final : public tr::TraverseListener
    {
    public:
        SwitchOriginalAndTranslation(const tr::eo::Sets2& aSets);
        void onText(const std::shared_ptr<tr::Text>&) override;
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
    private:
        const ProcessCm& processCm;
    };

    SwitchOriginalAndTranslation::SwitchOriginalAndTranslation(const tr::eo::Sets2& aSets)
        : processCm(*extractOriginalCm[aSets.comment]) {}

    void SwitchOriginalAndTranslation::onText(const std::shared_ptr<tr::Text>& text)
    {
        auto& tr = text->tr;
        tr.knownOriginal.reset();
        // Give some translation
        if (!tr.translation)
            tr.translation = tr.original;
        // Swap
        std::swap(tr.original, *tr.translation);
        // Process comments
        processCm.act(text->comm);
    }

    void SwitchOriginalAndTranslation::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& group)
    {
        processCm.act(group->comm);
    }

}   // anon namespace


void tr::switchOriginalAndTranslation(Project& prj, const eo::Sets2& sets)
{
    SwitchOriginalAndTranslation listener(sets);
    prj.traverse(listener, WalkOrder::ECONOMY, EnterMe::NO);
    prj.info.switchOriginalAndTranslation(sets.origPath);
    prj.fname.clear();
    prj.modify();
}

namespace {

    class ResetKnownOriginals final : public tr::TraverseListener
    {
    public:
        void onText(const std::shared_ptr<tr::Text>& x) override;
        size_t count() const { return n; }
    private:
        size_t n = 0;
    };

    void ResetKnownOriginals::onText(const std::shared_ptr<tr::Text>& x)
    {
        if (x->tr.knownOriginal) {
            x->tr.knownOriginal.reset();
            ++n;
        }
    }

}   // anon namespace

void tr::resetKnownOriginal(Project& prj)
{
    ResetKnownOriginals listener;
    prj.traverse(listener, WalkOrder::ECONOMY, EnterMe::NO);
    if (listener.count() != 0) {
        prj.stats(StatsMode::DIRECT, CascadeDropCache::NO);
        prj.modify();
    }
}


namespace {

    struct TwStats {
        size_t nTextsTouched = 0;
    };

    void twWalkFile(tr::File& prj, tr::File& ext,
                    const tr::tw::Sets& sets, TwStats& stats)
    {
        /// @todo [urgent] translate with original
    }

    void twWalkProject(tr::Project& prj, tr::Project& ext,
                       const tr::tw::Sets& sets, TwStats& stats)
    {
        for (auto& pSrcFile : prj.files) {
            if (auto pExtFile = ext.findFile(pSrcFile->id)) {
                twWalkFile(*pSrcFile, *pExtFile, sets, stats);
            }
        }
    }

}   // anon namespace


void tr::translateWithOriginal(Project& prj, const tw::Sets& sets)
{
    std::shared_ptr<Project> ext = Project::make();
    ext->load(sets.origPath);
    TwStats stats;
    twWalkProject(prj, *ext, sets, stats);
}
