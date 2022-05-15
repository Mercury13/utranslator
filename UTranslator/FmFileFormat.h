#pragma once

#include <QDialog>

// Libs
#include "u_Vector.h"
#include "u_Uptr.h"

// Qt ex
#include "QtMultiRadio.h"

// Translation
#include "TrDefines.h"

namespace Ui {
class FmFileFormat;
}

class FmAboutFormat;

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
            tf::LoadTextsSettings* loadSets,
            tf::SyncInfo* syncInfo,
            const tf::ProtoFilter& filter);
private slots:
    void comboChanged(int index);
    void reenable();
    void aboutFormat();
private:
    Ui::FmFileFormat *ui;
    EnlargingVector<const tf::FormatProto*> filteredProtos;
    Uptr<FmAboutFormat> fmAboutFormat;
    EcRadio<tf::LoadTo> radioLoadTo;
    EcRadio<tf::TextOwner> radioTextOwner;
    EcRadio<tf::Existing> radioExisting;

    using Super::exec;
    void collectFormats(
            const tf::FormatProto* myProto,
            const tf::ProtoFilter& filter);
    void copyFrom(const tf::FileFormat& fmt);
    void copyFrom(const tf::LoadTextsSettings* x);
    void copyFrom(const tf::SyncInfo* x);
    void copyTo(std::unique_ptr<tf::FileFormat>& r);
    void copyTo(tf::LoadTextsSettings* r);
    void copyTo(tf::SyncInfo* r);
    void reenableToFormat(const tf::FormatProto& proto);
    const tf::FormatProto* currentProto() const;
    escape::LineBreakMode escapeMode() const;
    escape::SpaceMode spaceMode() const;
};
