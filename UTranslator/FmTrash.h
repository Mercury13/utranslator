#ifndef FMTRASH_H
#define FMTRASH_H

#include <QDialog>

namespace Ui {
class FmTrash;
}

class FmTrash : public QDialog
{
    Q_OBJECT
public:
    explicit FmTrash(QWidget *parent = nullptr);
    ~FmTrash() override;

private:
    Ui::FmTrash *ui;
};

#endif // FMTRASH_H
