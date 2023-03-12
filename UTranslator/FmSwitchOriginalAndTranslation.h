#pragma once

#include <QDialog>

// Translation
#include "TrUtils.h"


namespace Ui {
class FmSwitchOriginalAndTranslation;
}

class FmSwitchOriginalAndTranslation : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmSwitchOriginalAndTranslation;
public:
    explicit FmSwitchOriginalAndTranslation(QWidget *parent = nullptr);
    ~FmSwitchOriginalAndTranslation() override;

    std::optional<tr::eo::Sets2> exec(bool doesOrigPathMean);
private:
    Ui::FmSwitchOriginalAndTranslation *ui;
    using Super::exec;
};
