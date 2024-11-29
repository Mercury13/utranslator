#include "FmTranslateWithOriginal.h"
#include "ui_FmTranslateWithOriginal.h"

FmTranslateWithOriginal::FmTranslateWithOriginal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmTranslateWithOriginal)
{
    ui->setupUi(this);
}

FmTranslateWithOriginal::~FmTranslateWithOriginal()
{
    delete ui;
}
