#include "FmXliff.h"
#include "ui_FmXliff.h"

#include "QtConsts.h"

FmXliff::FmXliff(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmXliff)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmXliff::~FmXliff()
{
    delete ui;
}

std::optional<xlf::Sets> FmXliff::exec(XliffMode mode, bool hasTranslation)
{
    bool isEx = (mode == XliffMode::EXPORT);
    // Available in export mode only
    ui->grpTexts->setEnabled(isEx);
    ui->chkWriteTranslation->setEnabled(hasTranslation);
    // Available in translate mode only
    ui->grpPrio->setEnabled(!isEx);
    // Go!
    if (Super::exec()) {
        xlf::Sets r;
        r.idSeparator = ui->edSeparator->text().toStdString();
        r.writeCdata = ui->chkCdata->isChecked();
        r.writeTranslation = ui->chkWriteTranslation->isChecked();
        return r;
    } else {
        return std::nullopt;
    }
}
