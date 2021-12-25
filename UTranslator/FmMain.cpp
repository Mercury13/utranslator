#include "FmMain.h"
#include "ui_FmMain.h"

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);

    // Setup layout
    ui->splitMain->setSizes({1, 1});

    // Stack
    ui->stackMain->setCurrentWidget(ui->pageStart);
}

FmMain::~FmMain()
{
    delete ui;
}
