#pragma once

#include <QDialog>

// Libs
#include "u_Vector.h"

#include "TrDefines.h"

namespace Ui {
class FmFileFormat;
}

class FmFileFormat : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmFileFormat;
public:
    explicit FmFileFormat(QWidget *parent = nullptr);
    ~FmFileFormat() override;

    bool exec(
            std::unique_ptr<tf::FileFormat>& x,
            const tf::ProtoFilter& filter);
private slots:
    void comboChanged(int index);
    void reenable();
private:
    Ui::FmFileFormat *ui;
    EnlargingVector<const tf::FormatProto*> filteredProtos;

    using Super::exec;
    void collectFormats(
            const tf::FormatProto* myProto,
            const tf::ProtoFilter& filter);
    void copyFrom(const tf::FileFormat& fmt);
    void copyTo(std::unique_ptr<tf::FileFormat>& r);
    void reenableToFormat(const tf::FormatProto& proto);
    tf::LineBreakEscapeMode escapeMode() const;
};
