#include "FmFind.h"
#include "ui_FmFind.h"

#include "QtConsts.h"

FmFind::FmFind(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmFind)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    ui->grpPlace->setEnabled(false);
}

FmFind::~FmFind()
{
    delete ui;
}

FindOptions FmFind::exec(tr::PrjType prjType)
{
    bool isTransl = false;
    switch (prjType) {
    case tr::PrjType::ORIGINAL:
        break;
    case tr::PrjType::FULL_TRANSL:
        isTransl = true;
        break;
    }
    ui->chkChanTranslation->setEnabled(isTransl);
    ui->chkChanTranslatorsComment->setEnabled(isTransl);
    ui->edFind->setFocus();

    FindOptions r;
    if (Super::exec()) {
        r.text = ui->edFind->text();
        r.channels.id = ui->chkChanId->isChecked();
        r.channels.original = ui->chkChanOriginal->isChecked();
        r.channels.authorsComment = ui->chkChanAuthorsComment->isChecked();
        r.channels.translation = ui->chkChanTranslation->isChecked();
        r.channels.translatorsComment = ui->chkChanTranslatorsComment->isChecked();
        r.options.matchCase = ui->chkMatchCase->isChecked();
    }
    return r;
}
