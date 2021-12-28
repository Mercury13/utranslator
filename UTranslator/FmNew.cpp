#include "FmNew.h"
#include "ui_FmNew.h"

FmNew::FmNew(QWidget *parent) :
    QDialog(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::FmNew)
{
    ui->setupUi(this);
    ui->edOrigLang->setCurrentText("en");
}

FmNew::~FmNew()
{
    delete ui;
}
