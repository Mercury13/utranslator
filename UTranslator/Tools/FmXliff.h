#ifndef FMXLIFF_H
#define FMXLIFF_H

#include <QDialog>

#include "TrXliff.h"
#include "QtMultiRadio.h"

namespace Ui {
class FmXliff;
}

enum class XliffMode : unsigned char {
    EXPORT, TRANSLATE };

class FmXliff : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmXliff;
public:
    explicit FmXliff(QWidget *parent = nullptr);
    ~FmXliff() override;
    std::optional<xlf::Sets> exec(XliffMode mode, bool hasTranslation);
private:
    Ui::FmXliff *ui;
    EcRadio<xlf::BadIdPolicy> radioPolicy;
    EcRadio<xlf::Priority> radioPriority;

    using Super::exec;
};

#endif // FMXLIFF_H
