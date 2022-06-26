#include "FmExtractOriginal.h"
#include "ui_FmExtractOriginal.h"

namespace {

    class TrOrig final : public ProcessTr {
    public:
        void act(tr::Translatable&) const override {};
    };

    class TrTransl final : public ProcessTr {
    public:
        void act(tr::Translatable& x) const override {
            x.original = x.translationSv();
        };
    };

    class TrTranslOrig final : public ProcessTr {
    public:
        void act(tr::Translatable& x) const override {
            if (x.translation)
                x.original = *x.translation;
        };
    };

    const TrOrig TR_ORIG;
    const TrTransl TR_TRANSL;
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
                x.authors = x.translators;
        };
    };

    class CmTransl final : public ProcessCm {
    public:
        void act(tr::Comments& x) const override
        {
            x.authors = x.translators;
        };
    };

    class CmTranslAuthor final : public ProcessCm {
    public:
        void act(tr::Comments& x) const override
        {
            if (!x.translators.empty())
                x.authors = x.translators;
        };
    };

    const CmAuthor CM_AUTHOR;
    const CmAuthorTransl CM_AUTHOR_TRANSL;
    const CmTransl CM_TRANSL;
    const CmAuthorTransl CM_TRANSL_AUTHOR;

}   // anon namespace

constinit const ec::Array<const ProcessTr*, TextChannel> extractOriginalTr {
    &TR_ORIG, &TR_TRANSL, &TR_TRANSL_ORIG
};

constinit const ec::Array<const ProcessCm*, CommentChannel> extractOriginalCm {
    &CM_AUTHOR, &CM_AUTHOR_TRANSL, &CM_TRANSL, &CM_TRANSL_AUTHOR
};


FmExtractOriginal::FmExtractOriginal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmExtractOriginal)
{
    ui->setupUi(this);

    radioText.setRadio(TextChannel::ORIG,        ui->radioChanOriginal);
    radioText.setRadio(TextChannel::TRANSL,      ui->radioChanTranslation);
    radioText.setRadio(TextChannel::TRANSL_ORIG, ui->radioChanTranslOrig);
    radioText.set(TextChannel::ORIG);

    radioComment.setRadio(CommentChannel::AUTHOR,        ui->radioCommAu);
    radioComment.setRadio(CommentChannel::AUTHOR_TRANSL, ui->radioCommAuTr);
    radioComment.setRadio(CommentChannel::TRANSL,        ui->radioCommTr);
    radioComment.setRadio(CommentChannel::TRANSL_AUTHOR, ui->radioCommTrAu);
    radioComment.set(CommentChannel::AUTHOR);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmExtractOriginal::~FmExtractOriginal()
{
    delete ui;
}

std::optional<ExtractOriginalSets> FmExtractOriginal::exec(int)
{
    if (Super::exec()) {
        return ExtractOriginalSets { radioText.get(), radioComment.get() };
    } else {
        return std::nullopt;
    }
}
