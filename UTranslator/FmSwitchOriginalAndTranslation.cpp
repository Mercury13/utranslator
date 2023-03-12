#include "FmSwitchOriginalAndTranslation.h"
#include "ui_FmSwitchOriginalAndTranslation.h"

#include "QtConsts.h"

FmSwitchOriginalAndTranslation::FmSwitchOriginalAndTranslation(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmSwitchOriginalAndTranslation)
{
    ui->setupUi(this);

    // OK/Cancel
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmSwitchOriginalAndTranslation::~FmSwitchOriginalAndTranslation()
{
    delete ui;
}

std::optional<tr::eo::Sets2> FmSwitchOriginalAndTranslation::exec(bool doesOrigPathMean)
{
    ui->edOrigFile->clear();
    ui->grpOriginal->setEnabled(doesOrigPathMean);
    if (Super::exec()) {
        tr::eo::Sets2 r;
        /// @todo [urgent] copy
        if (doesOrigPathMean) {
            r.origPath = ui->edOrigFile->text().toStdWString();
        }
        return r;
    } else {
        return std::nullopt;
    }
}
