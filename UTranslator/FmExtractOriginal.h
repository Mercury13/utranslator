#ifndef FMEXTRACTORIGINAL_H
#define FMEXTRACTORIGINAL_H

#include <QDialog>

namespace Ui {
class FmExtractOriginal;
}

class FmExtractOriginal : public QDialog
{
    Q_OBJECT

public:
    explicit FmExtractOriginal(QWidget *parent = nullptr);
    ~FmExtractOriginal();

private:
    Ui::FmExtractOriginal *ui;
};

#endif // FMEXTRACTORIGINAL_H
