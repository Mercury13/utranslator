#pragma once

#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

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

    std::optional<tr::eo::Sets2> exec(tr::Project& project);
private:
    Ui::FmSwitchOriginalAndTranslation *ui;
    EcRadio<tr::eo::Comment> radioComment;

    using Super::exec;
private slots:
    void chooseOriginal();
};
