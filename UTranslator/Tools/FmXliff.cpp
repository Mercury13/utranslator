#include "FmXliff.h"
#include "ui_FmXliff.h"

#include "QtConsts.h"

FmXliff::FmXliff(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmXliff)
{
    ui->setupUi(this);
    radioPolicy.setRadio(xlf::BadIdPolicy::KEEP, ui->radioKeep);
    radioPolicy.setRadio(xlf::BadIdPolicy::UNDERSCORE, ui->radioUnderscore);
    radioPriority.setRadio(xlf::Priority::PROJECT, ui->radioPrioThis);
    radioPriority.setRadio(xlf::Priority::XLIFF, ui->radioPrioXliff);
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
        r.id.badPolicy = radioPolicy.get(xlf::BadIdPolicy::UNDERSCORE);
        r.id.separator = ui->edSeparator->text().toStdString();
        r.writeText.cdata = ui->chkCdata->isChecked();
        r.writeText.translation = ui->chkWriteTranslation->isChecked();
        r.translate.priority = radioPriority.get(xlf::Priority::PROJECT);
        return r;
    } else {
        return std::nullopt;
    }
}
