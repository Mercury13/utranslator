#ifndef FMABOUTFORMAT_H
#define FMABOUTFORMAT_H

#include <QDialog>

#include "TrDefines.h"

namespace Ui {
class FmAboutFormat;
}

class FmAboutFormat : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmAboutFormat;
public:
    explicit FmAboutFormat(QWidget *parent = nullptr);
    ~FmAboutFormat();

    void exec(const tf::FormatProto* proto);

private:
    Ui::FmAboutFormat *ui;
    using Super::exec;
};

#endif // FMABOUTFORMAT_H
