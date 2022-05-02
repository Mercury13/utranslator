#include "FmNew.h"
#include "ui_FmNew.h"

#include <QDialogButtonBox>

// Qt ex
#include "QtConsts.h"


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
    ui->edOrigLang->setCurrentText("en");

    // Accept/reject
    connect(ui->btCancel, &QPushButton::clicked, this, &This::reject);
    connect(ui->btBack, &QPushButton::clicked, wizard.get(), &WizardManager::goBack);
    connect(ui->btNext, &QPushButton::clicked, this, &This::nextPressed);
}


FmNew::~FmNew()
{
    delete ui;
}


void FmNew::nextPressed()
{
    switch (wizard->index()) {
    case 0:
        wizard->goNext(ui->pageOriginal, false);
        break;
    case 1:
        accept();
        break;
    }
}


std::unique_ptr<tr::PrjInfo> FmNew::exec(int)
{
    // Start wizard
    ui->stackWizard->setCurrentWidget(ui->pageType);
    if (!Super::exec()) return {};
    auto r = std::make_unique<tr::PrjInfo>();
    copyTo(*r);
    return r;
}


void FmNew::copyTo(tr::PrjInfo& r)
{
    /// @todo [transl] Translation is not implemented in FmNew
    r.type = tr::PrjType::ORIGINAL;
    r.orig.lang = ui->edOrigLang->currentText().toStdString();
    r.transl.lang.clear();
}
