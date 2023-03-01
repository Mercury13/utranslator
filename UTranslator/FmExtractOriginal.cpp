#include "FmExtractOriginal.h"
#include "ui_FmExtractOriginal.h"

namespace {

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

    constinit const CmAuthor CM_AUTHOR;
    constinit const CmAuthorTransl CM_AUTHOR_TRANSL;
    constinit const CmTransl CM_TRANSL;
    constinit const CmAuthorTransl CM_TRANSL_AUTHOR;

}   // anon namespace

constinit const ec::Array<const ProcessTr*, tr::eo::Text> extractOriginalTr {
    &TR_ORIG, &TR_TRANSL_ORIG
};

constinit const ec::Array<const ProcessCm*, tr::eo::Comment> extractOriginalCm {
    &CM_AUTHOR, &CM_AUTHOR_TRANSL, &CM_TRANSL, &CM_TRANSL_AUTHOR
};


FmExtractOriginal::FmExtractOriginal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmExtractOriginal)
{
    ui->setupUi(this);

    radioText.setRadio(tr::eo::Text::ORIG,        ui->radioChanOriginal);
    radioText.setRadio(tr::eo::Text::TRANSL_ORIG, ui->radioChanTranslOrig);
    radioText.set(tr::eo::Text::ORIG);

    radioComment.setRadio(tr::eo::Comment::AUTHOR,        ui->radioCommAu);
    radioComment.setRadio(tr::eo::Comment::AUTHOR_TRANSL, ui->radioCommAuTr);
    radioComment.setRadio(tr::eo::Comment::TRANSL,        ui->radioCommTr);
    radioComment.setRadio(tr::eo::Comment::TRANSL_AUTHOR, ui->radioCommTrAu);
    radioComment.set(tr::eo::Comment::AUTHOR);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmExtractOriginal::~FmExtractOriginal()
{
    delete ui;
}

std::optional<tr::eo::Sets> FmExtractOriginal::exec(int)
{
    if (Super::exec()) {
        return tr::eo::Sets { radioText.get(), radioComment.get() };
    } else {
        return std::nullopt;
    }
}
