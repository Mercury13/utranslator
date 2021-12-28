#ifndef FMNEW_H
#define FMNEW_H

#include <QDialog>

namespace Ui {
class FmNew;
}

class FmNew : public QDialog
{
    Q_OBJECT

public:
    explicit FmNew(QWidget *parent = nullptr);
    ~FmNew();

private:
    Ui::FmNew *ui;
};

#endif // FMNEW_H
