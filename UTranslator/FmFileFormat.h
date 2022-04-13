#ifndef FMFILEFORMAT_H
#define FMFILEFORMAT_H

#include <QDialog>

namespace Ui {
class FmFileFormat;
}

class FmFileFormat : public QDialog
{
    Q_OBJECT

public:
    explicit FmFileFormat(QWidget *parent = nullptr);
    ~FmFileFormat();

private:
    Ui::FmFileFormat *ui;
};

#endif // FMFILEFORMAT_H
