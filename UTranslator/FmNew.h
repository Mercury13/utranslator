#ifndef FMNEW_H
#define FMNEW_H

#include <QDialog>
#include "TrDefines.h"

namespace Ui {
class FmNew;
}

class FmNew : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit FmNew(QWidget *parent = nullptr);
    ~FmNew() override;
    std::unique_ptr<tr::PrjInfo> exec(int dummy=0);
protected:
private:
    Ui::FmNew *ui;

    using Super::exec;
    void copyTo(tr::PrjInfo&);
};

#endif // FMNEW_H
