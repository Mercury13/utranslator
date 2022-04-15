#pragma once

#include <QDialog>

#include "TrDefines.h"

namespace Ui {
class FmFileFormat;
}

class FmFileFormat : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit FmFileFormat(QWidget *parent = nullptr);
    ~FmFileFormat() override;

    bool exec(CloningUptr<tf::FileFormat>& x);
private:
    Ui::FmFileFormat *ui;

    using Super::exec;
    void copyFrom(Flags<tf::Usfg> flags, const tf::UnifiedSets& x);
    void copyTo(tf::UnifiedSets& x);
};
