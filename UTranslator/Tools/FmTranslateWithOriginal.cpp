#include "FmTranslateWithOriginal.h"
#include "ui_FmTranslateWithOriginal.h"

#include "QtConsts.h"

FmTranslateWithOriginal::FmTranslateWithOriginal(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmTranslateWithOriginal)
{
    ui->setupUi(this);

    radioPriority.setRadio(tr::tw::Priority::EXISTING, ui->radioPrioThis);
    radioPriority.setRadio(tr::tw::Priority::EXTERNAL, ui->radioPrioExt );
    radioPriority.set(tr::tw::Priority::EXISTING);
}

FmTranslateWithOriginal::~FmTranslateWithOriginal()
{
    delete ui;
}

std::optional<tr::tw::Sets> FmTranslateWithOriginal::exec(int)
{
    if (!Super::exec())
        return std::nullopt;
    tr::tw::Sets r;
    r.holeSigns.originalIsTranslation = ui->chkIsOriginal->isChecked();
    r.priority = radioPriority.get();
    return r;
}
