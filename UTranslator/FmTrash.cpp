#include "FmTrash.h"
#include "ui_FmTrash.h"

FmTrash::FmTrash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmTrash)
{
    ui->setupUi(this);
}

FmTrash::~FmTrash()
{
    delete ui;
}
