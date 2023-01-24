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
    connect(ui->btBrowseFile, &QAbstractButton::clicked, this, &This::chooseOriginal);
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
    ui->edTranslLanguage->setCurrentText(str::toQ(x.transl.lang));
    ui->chkPseudoLoc->setChecked(x.transl.wantPseudoLoc);
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
        r.transl.wantPseudoLoc = ui->chkPseudoLoc->isChecked();
    }
}


void FmProjectSettings::chooseOriginal()
{
    filedlg::Filters filters {
        FILTER_TRANSLATABLE, filedlg::ALL_FILES,
    };
    auto fname = filedlg::open(
                this, L"Choose original", filters, WEXT_TRANSLATABLE,
                filedlg::AddToRecent::NO);
    if (!fname.empty()) {
        ui->edOrigFile->setText(QString::fromStdWString(fname));
    }
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
