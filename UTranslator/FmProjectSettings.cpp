#include "FmProjectSettings.h"
#include "ui_FmProjectSettings.h"

#include "QtConsts.h"

FmProjectSettings::FmProjectSettings(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmProjectSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmProjectSettings::~FmProjectSettings()
{
    delete ui;
}

void FmProjectSettings::copyFrom(const tr::PrjInfo& x)
{
}

void FmProjectSettings::copyTo(tr::PrjInfo& r)
{

}

bool FmProjectSettings::exec(tr::PrjInfo& info)
{
    copyFrom(info);
    bool b = Super::exec();
    if (b) {
        copyTo(info);
    }
    return b;
}
