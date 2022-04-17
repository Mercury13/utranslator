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
    ui->chkPlaceTranslation->setEnabled(isTransl);
    ui->chkPlaceTranslatorsComment->setEnabled(isTransl);
    ui->edFind->setFocus();

    FindOptions r;
    if (Super::exec()) {
        /// @todo [urgent] copy
    }
    return r;
}
