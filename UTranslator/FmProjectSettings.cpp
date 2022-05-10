#include "FmProjectSettings.h"
#include "ui_FmProjectSettings.h"

// Qt ex
#include "QtConsts.h"

// Libs
#include "u_Qstrings.h"

// Project-local
#include "d_Strings.h"


namespace {

    void fillLangs(QComboBox* x)
    {
        for (auto& v : langList) {
            x->addItem(str::toQ(v));
        }
    }

}   // anon namespace


FmProjectSettings::FmProjectSettings(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmProjectSettings)
{
    ui->setupUi(this);

    fillLangs(ui->edOrigLanguage);
    fillLangs(ui->edTranslLanguage);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmProjectSettings::~FmProjectSettings()
{
    delete ui;
}

void FmProjectSettings::copyFrom(const tr::PrjInfo& x)
{
    // ORIG: Language
        // NOT canEditOriginal() — in freestyle project you can add files and edit this
    ui->wiOrigLanguage->setEnabled(x.canAddFiles());
    ui->edOrigLanguage->setCurrentText(str::toQ(x.orig.lang));
    // ORIG: File
    ui->wiOrigFile->setEnabled(x.hasOriginalPath());
    ui->edOriginalFile->setText(str::toQ(x.orig.absPath.wstring()));

    // TRANSL: Language
    ui->grpTranslation->setEnabled(x.isTranslation());
    ui->edTranslLanguage->setCurrentText(str::toQ(x.transl.lang));
}

void FmProjectSettings::copyTo(tr::PrjInfo& r)
{
    // ORIG: language
    if (r.canAddFiles()) {
        r.orig.lang = ui->edOrigLanguage->currentText().toStdString();
    }
    // ORIG: file
    if (r.hasOriginalPath()) {
        r.orig.absPath = ui->edOriginalFile->text().toStdWString();
    }

    // TRANSL: language
    if (r.isTranslation()) {
        r.transl.lang = ui->edTranslLanguage->currentText().toStdString();
    }
}

bool FmProjectSettings::exec(tr::PrjInfo& info)
{
    copyFrom(info);
    bool b = Super::exec();
    if (b) {
        copyTo(info);
    }
    return b;
}
