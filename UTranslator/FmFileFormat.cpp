#include "FmFileFormat.h"
#include "ui_FmFileFormat.h"

FmFileFormat::FmFileFormat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmFileFormat)
{
    ui->setupUi(this);
}

FmFileFormat::~FmFileFormat()
{
    delete ui;
}
