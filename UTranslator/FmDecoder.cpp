#include "FmDecoder.h"
#include "ui_FmDecoder.h"

// Qt
#include <QDialogButtonBox>

// Qt ex
#include "QtConsts.h"

// Decoder
#include "Decoders.h"

FmDecoder::FmDecoder(QWidget *parent) :
    QDialog(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmDecoder)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    connect(ui->btDecodeC, &QPushButton::clicked, this, &This::decodeCpp);
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


void FmDecoder::decodeCpp()
{
    auto text = ui->memo->toPlainText().toStdU32String();
    auto result = decode::cpp(text);
    if (result != text) {
        auto text2 = QString::fromStdU32String(result);
        ui->memo->setPlainText(text2);
    }
    ui->lbDescription->setText("Decode C++ strings: \"first\\nsecond\" → first⤶second");
}
