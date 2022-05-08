#ifndef FMPROJECTSETTINGS_H
#define FMPROJECTSETTINGS_H

#include <QDialog>

#include "TrDefines.h"

namespace Ui {
class FmProjectSettings;
}

class FmProjectSettings : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmProjectSettings;
public:
    explicit FmProjectSettings(QWidget *parent = nullptr);
    ~FmProjectSettings() override;

    bool exec(tr::PrjInfo& info);

private:
    Ui::FmProjectSettings *ui;
    void copyFrom(const tr::PrjInfo& x);
    void copyTo(tr::PrjInfo& r);
    using Super::exec;
};

#endif // FMPROJECTSETTINGS_H
