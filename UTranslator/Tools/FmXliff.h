#ifndef FMXLIFF_H
#define FMXLIFF_H

#include <QDialog>

namespace Ui {
class FmXliff;
}

enum class XliffMode : unsigned char {
    EXPORT, TRANSLATE };

class FmXliff : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit FmXliff(QWidget *parent = nullptr);
    ~FmXliff() override;
    int exec(XliffMode mode);
private:
    Ui::FmXliff *ui;
    using Super::exec;
};

#endif // FMXLIFF_H
