#ifndef FMTRANSLATEWITHORIGINAL_H
#define FMTRANSLATEWITHORIGINAL_H

// Qt
#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

// STL
#include <optional>

// Translation
#include "TrUtils.h"

namespace Ui {
class FmTranslateWithOriginal;
}

class FmTranslateWithOriginal : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmTranslateWithOriginal;
public:
    explicit FmTranslateWithOriginal(QWidget *parent = nullptr);
    ~FmTranslateWithOriginal() override;;

    std::optional<tr::tw::Sets> exec(int dummy = 0);

private:
    Ui::FmTranslateWithOriginal *ui;
    EcRadio<tr::tw::Priority> radioPriority;

    using Super::exec;
};

#endif // FMTRANSLATEWITHORIGINAL_H
