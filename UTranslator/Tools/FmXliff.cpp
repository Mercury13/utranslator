#include "FmXliff.h"
#include "ui_FmXliff.h"

#include "QtConsts.h"

FmXliff::FmXliff(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmXliff)
{
    ui->setupUi(this);
}

FmXliff::~FmXliff()
{
    delete ui;
}

std::optional<tr::XliffSets> FmXliff::exec(XliffMode mode, bool hasTranslation)
{
    bool isEx = (mode == XliffMode::EXPORT);
    // Available in export mode only
    ui->grpTexts->setEnabled(isEx);
    ui->chkWriteTranslation->setEnabled(hasTranslation);
    // Available in translate mode only
    ui->grpPrio->setEnabled(!isEx);
    // Go!
    if (Super::exec()) {
        tr::XliffSets r;
        /// @todo [urgent] copy to r
        r.idSeparator = ui->edSeparator->text().toStdString();
        r.writeCdata = ui->chkCdata->isChecked();
        r.writeTranslation = ui->chkWriteTranslation->isChecked();
        return r;
    } else {
        return std::nullopt;
    }
}
