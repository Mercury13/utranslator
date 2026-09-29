#ifndef FMXLIFF_H
#define FMXLIFF_H

#include <QDialog>

namespace Ui {
class FmXliff;
}

class FmXliff : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit FmXliff(QWidget *parent = nullptr);
    ~FmXliff();

private:
    Ui::FmXliff *ui;
};

#endif // FMXLIFF_H
