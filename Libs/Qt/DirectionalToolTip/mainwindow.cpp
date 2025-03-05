#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "BalloonTip.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    BalloonTip::showBalloon(QMessageBox::Warning, "Test", "Hello World", QCursor::pos(), 10000,
                            static_cast<BalloonDir>(ui->spinBox->value()));
}

void MainWindow::on_btFlip_clicked()
{
    auto layDir = QApplication::layoutDirection();
    if (layDir == Qt::RightToLeft) {
        layDir = Qt::LeftToRight;
    } else {
        layDir = Qt::RightToLeft;
    }
    QApplication::setLayoutDirection(layDir);
}

