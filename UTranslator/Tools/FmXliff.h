#ifndef FMXLIFF_H
#define FMXLIFF_H

#include <QDialog>

#include "TrXliff.h"

namespace Ui {
class FmXliff;
}

namespace tr {
    struct XliffSets;
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
    std::optional<tr::XliffSets> exec(XliffMode mode, bool hasTranslation);
private:
    Ui::FmXliff *ui;
    using Super::exec;
};

#endif // FMXLIFF_H
