#include "FmNew.h"
#include "ui_FmNew.h"

#include <QDialogButtonBox>

// Qt ex
#include "QtConsts.h"

FmNew::FmNew(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmNew)
{
    ui->setupUi(this);

    // radioType
    radioType.setRadio(tr::PrjType::ORIGINAL, ui->radioOriginal);
    radioType.setRadio(tr::PrjType::FULL_TRANSL, ui->radioTranslation);
    radioType.set(tr::PrjType::ORIGINAL);

    // Initial setup
    ui->edOrigLang->setCurrentText("en");

    // Accept/reject
    connect(ui->btCancel, &QPushButton::clicked, this, &This::reject);
    //connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    //connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmNew::~FmNew()
{
    delete ui;
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
