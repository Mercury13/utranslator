#include "FmNew.h"
#include "ui_FmNew.h"

#include <QDialogButtonBox>
#include <QMessageBox>

// Qt ex
#include "QtConsts.h"
#include "i_OpenSave.h"

// Libs
#include "u_Qstrings.h"
#include "d_Strings.h"


///// DblClickRadio ////////////////////////////////////////////////////////////


void DblClickRadio::mouseDoubleClickEvent(QMouseEvent *event)
{
    Super::mouseDoubleClickEvent(event);
    emit doubleClicked();
}


///// WizardManager ////////////////////////////////////////////////////////////


WizardManager::WizardManager(
        QStackedWidget* aStack, QWidget* aStartingPage,
        QAbstractButton* abtBack, QAbstractButton* abtNext,
        const QString& asFinish)
    : stack(aStack), startingPage(aStartingPage),
      btBack(abtBack), btNext(abtNext), sFinish(asFinish)
{
    sNext = abtNext->text();
}


void WizardManager::updateButtons()
{
    btBack->setEnabled(hasBack());
    if (hasNext()) {
        btNext->setText(sNext);
    } else {
        btNext->setText(sFinish);
    }
}


void WizardManager::start()
{
    history.clear();
    fHasNext = true;
    stack->setCurrentWidget(startingPage);
    updateButtons();
}


QWidget* WizardManager::page() const { return stack->currentWidget(); }


void WizardManager::goNext(QWidget* newPage, bool hasNext)
{
    history.push_back(page());
    fHasNext = hasNext;
    stack->setCurrentWidget(newPage);
    updateButtons();
}


void WizardManager::goBack()
{
    if (history.empty())
        return;
    auto lastPage = history.back();
    history.pop_back();
    stack->setCurrentWidget(lastPage);
    fHasNext = true;
    updateButtons();
}


///// FmNew ////////////////////////////////////////////////////////////////////


void FmNew::fillLangs(QComboBox* x, const char* defaultText)
{
    for (auto& v : langList) {
        x->addItem(str::toQ(v));
    }
    x->setCurrentText(defaultText);
}


FmNew::FmNew(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmNew)
{
    ui->setupUi(this);
    wizard = std::make_unique<WizardManager>(
                ui->stackWizard, ui->pageType,
                ui->btBack, ui->btNext, "Finish");

    // radioType
    radioType.setRadio(tr::PrjType::ORIGINAL, ui->radioOriginal);
    radioType.setRadio(tr::PrjType::FULL_TRANSL, ui->radioTranslation);
    radioType.set(tr::PrjType::ORIGINAL);

    // Initial setup
    fillLangs(ui->edOriginal,    "en");
    fillLangs(ui->edTranslation, "de");

    // Radio double clicks
    connect(ui->radioOriginal, &DblClickRadio::doubleClicked, this, &This::nextPressed);
    connect(ui->radioTranslation, &DblClickRadio::doubleClicked, this, &This::nextPressed);
    // Accept/reject
    connect(ui->btCancel, &QPushButton::clicked, this, &This::reject);
    connect(ui->btBack, &QPushButton::clicked, wizard.get(), &WizardManager::goBack);
    connect(ui->btNext, &QPushButton::clicked, this, &This::nextPressed);
}


FmNew::~FmNew()
{
    delete ui;
}


bool FmNew::chooseOriginal()
{
    filedlg::Filters filters {
        FILTER_TRANSLATABLE, filedlg::ALL_FILES,
    };
    auto fname = filedlg::open(this, L"Choose original", filters, WEXT_TRANSLATABLE,
                                filedlg::AddToRecent::NO);
    if (fname.empty())
        return false;
    try {
        project = tr::Project::make();
        project->load(fname);
        project->info.type = tr::PrjType::FULL_TRANSL;
        project->info.orig.absPath = std::filesystem::weakly_canonical(fname);
        return true;
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Original", e.what());
        return false;
    }
}


void FmNew::reenableSettingsPage()
{
    auto type = radioType.get();
    if (project) {
        ui->edOriginal->setCurrentText(QString::fromStdString(project->info.orig.lang));
    }
    ui->wiOriginal->setEnabled(tr::PrjInfo::canAddFiles(type));
    ui->wiTranslation->setVisible(tr::PrjInfo::isTranslation(type));
}


void FmNew::nextPressed()
{

    switch (wizard->index()) {
    case 0:
        switch (radioType.get()) {
        case tr::PrjType::ORIGINAL:
            break;
        case tr::PrjType::FULL_TRANSL:
            if (!chooseOriginal())
                return;
            break;
        }
        wizard->goNext(ui->pageOriginal, false);
        reenableSettingsPage();
        break;
    case 1:
        accept();
        break;
    }
}


std::shared_ptr<tr::Project> FmNew::exec(std::u8string_view defaultFile)
{
    project.reset();
    wizard->start();
    if (!Super::exec()) return {};
    tr::PrjInfo info;
    copyTo(info);
    switch (info.type) {
    case tr::PrjType::ORIGINAL: {
            // Create a project from scratch
            auto x = tr::Project::make(std::move(info));
            x->addFile(defaultFile, tr::Modify::NO);
            return x;
        }
    case tr::PrjType::FULL_TRANSL:
        info.orig.absPath = std::move(project->info.orig.absPath);
        project->info = std::move(info);
        project->fname.clear();
        return std::move(project);
    }
    // Strange type
    return {};
}


void FmNew::copyTo(tr::PrjInfo& r)
{
    r.type = radioType.get();
    r.orig.lang = ui->edOriginal->currentText().toStdString();
    if (tr::PrjInfo::isTranslation(r.type)) {
        r.transl.lang = ui->edTranslation->currentText().toStdString();
    } else {
        r.transl.lang.clear();
    }
}
