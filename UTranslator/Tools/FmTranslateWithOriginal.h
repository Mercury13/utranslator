#ifndef FMTRANSLATEWITHORIGINAL_H
#define FMTRANSLATEWITHORIGINAL_H

#include <QDialog>

namespace Ui {
class FmTranslateWithOriginal;
}

class FmTranslateWithOriginal : public QDialog
{
    Q_OBJECT

public:
    explicit FmTranslateWithOriginal(QWidget *parent = nullptr);
    ~FmTranslateWithOriginal();

private:
    Ui::FmTranslateWithOriginal *ui;
};

#endif // FMTRANSLATEWITHORIGINAL_H
