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

int FmXliff::exec(XliffMode mode)
{
    bool isEx = (mode == XliffMode::EXPORT);
    ui->grpTexts->setEnabled(isEx);
    return Super::exec();
}
