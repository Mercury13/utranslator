#include "FmExtractOriginal.h"
#include "ui_FmExtractOriginal.h"

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
