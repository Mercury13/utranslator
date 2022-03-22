// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Project
#include "TrProject.h"

// UI forms
#include "FmNew.h"

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

void FmMain::doNew()
{
    if (auto result = fmNew.ensure(this).exec(0)) {
        project = std::make_shared<tr::Project>(std::move(*result));
        ui->stackMain->setCurrentWidget(ui->pageMain);
    }
}
