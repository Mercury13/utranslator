#include "FmFileFormat.h"
#include "ui_FmFileFormat.h"

#include "TrFile.h"

FmFileFormat::FmFileFormat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmFileFormat)
{
    ui->setupUi(this);
}

FmFileFormat::~FmFileFormat()
{
    delete ui;
}

void FmFileFormat::copyFrom(Flags<tf::Usfg> flags, const tf::UnifiedSets& x)
{
    // Enable-disable
    ui->grpText->setEnabled(flags.have(tf::Usfg::TEXT_FORMAT));
    ui->wiLineBreaksInStrings->setEnabled(flags.have(tf::Usfg::TEXT_ESCAPE));
    ui->grpMultitier->setEnabled(flags.have(tf::Usfg::MULTITIER));
}

void FmFileFormat::copyTo(tf::UnifiedSets& x)
{
    /// @todo [urgent] copyTo
}

bool FmFileFormat::exec(CloningUptr<tf::FileFormat>& x)
{
    auto& obj = x ? *x : tf::Dummy::INST;
    copyFrom(obj.proto().setsFlags(), obj.unifiedSets());
    bool r = Super::exec();
    if (r) {
        tf::UnifiedSets newSets;
        copyTo(newSets);
    }
    return r;
}
