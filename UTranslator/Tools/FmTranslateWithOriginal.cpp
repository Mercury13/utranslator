#include "FmTranslateWithOriginal.h"
#include "ui_FmTranslateWithOriginal.h"

// Qt
#include <QDialogButtonBox>

// Qt ex
#include "QtConsts.h"

FmTranslateWithOriginal::FmTranslateWithOriginal(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmTranslateWithOriginal)
{
    ui->setupUi(this);

    radioPriority.setRadio(tr::tw::Priority::EXISTING, ui->radioPrioThis);
    radioPriority.setRadio(tr::tw::Priority::EXTERNAL, ui->radioPrioExt );
    radioPriority.set(tr::tw::Priority::EXISTING);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmTranslateWithOriginal::~FmTranslateWithOriginal()
{
    delete ui;
}

std::optional<tr::tw::Sets> FmTranslateWithOriginal::exec(int)
{
    ui->chkIsOriginal->setFocus();
    if (!Super::exec())
        return std::nullopt;
    tr::tw::Sets r;
    r.holeSigns.origIsExt = ui->chkIsOriginal->isChecked();
    r.holeSigns.emptyString = ui->chkEmptyString->isChecked();
    r.priority = radioPriority.get();
    return r;
}
