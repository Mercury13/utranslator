#include "FmDecoder.h"
#include "ui_FmDecoder.h"

// Qt
#include <QDialogButtonBox>

// Qt ex
#include "QtConsts.h"

FmDecoder::FmDecoder(QWidget *parent) :
    QDialog(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmDecoder)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmDecoder::~FmDecoder()
{
    delete ui;
}


int FmDecoder::exec()
{
    ui->lbDescription->setText(u8"Press one of buttons to see description and convert text ➤➤➤");
    /// @todo [urgent] copy some text?
    return Super::exec();
}
