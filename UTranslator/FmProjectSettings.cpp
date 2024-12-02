#include "FmProjectSettings.h"
#include "ui_FmProjectSettings.h"

// Qt ex
#include "QtConsts.h"
#include "i_OpenSave.h"

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
    connect(ui->btBrowseOriginal, &QAbstractButton::clicked, this, &This::browseOriginal);
    connect(ui->btBrowseReference, &QAbstractButton::clicked, this, &This::browseReference);
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
    ui->edOrigFile->setText(str::toQ(x.orig.absPath.wstring()));

    // TRANSL: Language
    ui->grpTranslation->setEnabled(x.isTranslation());
    ui->wiReference->setEnabled(x.canHaveReference());
    ui->edTranslLanguage->setCurrentText(str::toQ(x.transl.lang));
    ui->edReference->setText(str::toQ(x.ref.absPath.wstring()));
    ui->chkPseudoLoc->setChecked(x.transl.pseudoloc.isOn);
}

void FmProjectSettings::copyTo(tr::PrjInfo& r)
{
    // ORIG: language
    if (r.canAddFiles()) {
        r.orig.lang = ui->edOrigLanguage->currentText().toStdString();
    }
    // ORIG: file
    if (r.hasOriginalPath()) {
        r.orig.absPath = ui->edOrigFile->text().toStdWString();
    }

    // TRANSL: language
    if (r.isTranslation()) {
        r.transl.lang = ui->edTranslLanguage->currentText().toStdString();
        r.transl.pseudoloc.isOn = ui->chkPseudoLoc->isChecked();
        if (r.canHaveReference()) {
            r.ref.absPath = ui->edReference->text().toStdWString();
        }
    }
}


void FmProjectSettings::browseOriginal()
{
    filedlg::browseLineEdit(this,  L"Choose original",
                FILTER_TRANSLATABLE, WEXT_TRANSLATABLE, ui->edOrigFile);
}

void FmProjectSettings::browseReference()
{
    filedlg::browseLineEdit(this,  L"Choose reference translation",
                FILTER_TRANSLATION, WEXT_TRANSLATION, ui->edReference);
}

bool FmProjectSettings::exec(tr::PrjInfo& info)
{
    copyFrom(info);
    bool b = Super::exec();
    if (b) {
        auto info1 = info;
        copyTo(info1);
        if (info1 != info) {
            info = std::move(info1);
            return true;
        }
    }
    return false;
}
