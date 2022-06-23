#include "FmExtractOriginal.h"
#include "ui_FmExtractOriginal.h"

FmExtractOriginal::FmExtractOriginal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmExtractOriginal)
{
    ui->setupUi(this);
}

FmExtractOriginal::~FmExtractOriginal()
{
    delete ui;
}
