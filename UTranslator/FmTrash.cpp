#include "FmTrash.h"
#include "ui_FmTrash.h"

#include "QtConsts.h"

FmTrash::FmTrash(QWidget *parent) :
    QDialog(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmTrash)
{
    ui->setupUi(this);
}

FmTrash::~FmTrash()
{
    delete ui;
}
