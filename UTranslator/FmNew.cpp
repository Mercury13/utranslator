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

    // Initial setup
    ui->edOrigLang->setCurrentText("en");

    // Accept/reject
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmNew::~FmNew()
{
    delete ui;
}

std::unique_ptr<tr::PrjInfo> FmNew::exec(int)
{
    if (!Super::exec()) return {};
    auto r = std::make_unique<tr::PrjInfo>();
    copyTo(*r);
    return r;
}


void FmNew::copyTo(tr::PrjInfo& r)
{
    /// @todo [transl] Right now original only
    r.type = tr::PrjType::ORIGINAL;
    r.orig.lang = ui->edOrigLang->currentText().toStdString();
    r.transl.lang.clear();
}
