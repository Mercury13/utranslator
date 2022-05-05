#include "WiFind.h"
#include "ui_WiFind.h"

WiFind::WiFind(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WiFind)
{
    ui->setupUi(this);
}

WiFind::~WiFind()
{
    delete ui;
}
