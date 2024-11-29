#include "FmExtractOriginal.h"
#include "ui_FmExtractOriginal.h"


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
