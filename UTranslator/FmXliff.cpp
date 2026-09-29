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
