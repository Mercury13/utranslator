#include "FmSwitchOriginalAndTranslation.h"
#include "ui_FmSwitchOriginalAndTranslation.h"

// Qt
#include <QToolButton>

// Qt ex
#include "QtConsts.h"

// Misc libs
#include "i_OpenSave.h"

// Translation
#include "TrProject.h"

// Other project-local
#include "d_Strings.h"

FmSwitchOriginalAndTranslation::FmSwitchOriginalAndTranslation(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmSwitchOriginalAndTranslation)
{
    ui->setupUi(this);

    // Status
    ui->imgOk ->load(QString{":/Status/ok.svg"  });
    ui->imgBad->load(QString{":/Status/stop.svg"});

    // Original
    connect(ui->btBrowseFile, &QToolButton::clicked, this, &This::chooseOriginal);

    // Comments
    radioComment.setRadio(tr::eo::Comment::AUTHOR,        ui->radioAu);
    radioComment.setRadio(tr::eo::Comment::AUTHOR_TRANSL, ui->radioAuTr);
    radioComment.setRadio(tr::eo::Comment::TRANSL,        ui->radioTr);
    radioComment.setRadio(tr::eo::Comment::TRANSL_AUTHOR, ui->radioTrAu);
    radioComment.set(tr::eo::Comment::AUTHOR);

    // OK/Cancel
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmSwitchOriginalAndTranslation::~FmSwitchOriginalAndTranslation()
{
    delete ui;
}

std::optional<tr::eo::Sets2> FmSwitchOriginalAndTranslation::exec(tr::Project& project)
{
    bool hasPath = project.info.hasOriginalPath();
    ui->edOrigFile->clear();
    ui->grpOriginal->setEnabled(hasPath);
    auto stats = project.stats(tr::StatsMode::DIRECT, tr::CascadeDropCache::YES);
    if (stats.text.nAutoProblem == 0) {
        ui->imgOk->show();
        ui->imgBad->hide();
        ui->lbStatus->setText("No auto-detected problems, go on!");
    } else {
        ui->imgOk->hide();
        ui->imgBad->show();
        if (stats.text.nUntranslated != 0) {
            ui->lbStatus->setText("You’d better translate all texts.");
        } else {
            ui->lbStatus->setText("Changed originals will be reset. You’d better fix them.");
        }
    }
    if (Super::exec()) {
        tr::eo::Sets2 r;
        r.comment = radioComment.get();
        if (hasPath) {
            r.origPath = ui->edOrigFile->text().toStdWString();
        }
        return r;
    } else {
        return std::nullopt;
    }
}


void FmSwitchOriginalAndTranslation::chooseOriginal()
{
    filedlg::Filters filters {
        FILTER_TRANSLATABLE, filedlg::ALL_FILES,
    };
    auto fname = filedlg::open(
                this, L"Choose new original", filters, WEXT_TRANSLATABLE,
                filedlg::AddToRecent::NO);
    if (!fname.empty()) {
        ui->edOrigFile->setText(QString::fromStdWString(fname));
    }
}
