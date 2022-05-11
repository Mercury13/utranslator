#include "FmAboutFormat.h"
#include "ui_FmAboutFormat.h"

// Libs
#include "u_Qstrings.h"
#include "QtConsts.h"

FmAboutFormat::FmAboutFormat(QWidget *parent) :
    Super(parent, QDlgType::FIXED),
    ui(new Ui::FmAboutFormat)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
}

FmAboutFormat::~FmAboutFormat()
{
    delete ui;
}


void FmAboutFormat::exec(const tf::FormatProto* proto)
{
    ui->lbNameValue->setText(str::toQ(proto->locName()));
    auto ws = proto->workingSets();
    auto caps = proto->caps();
    if (proto->isDummy()) {
        ui->lbTypeValue->setText(u8"—");
    } else if (caps.have(tf::Fcap::XML)) {
        ui->lbTypeValue->setText("XML");
    } else if (ws.have(tf::Usfg::TEXT_FORMAT)) {
        ui->lbTypeValue->setText("Text");
    } else {
        ui->lbTypeValue->setText("Binary");
    }
    ui->lbSoftwareValue->setText(str::toQ(proto->locSoftware()));
    ui->lbIdTypeValue->setText(str::toQ(proto->locIdType()));
    QString text;
    if (caps.haveAll(tf::Fcap::IMPORT | tf::Fcap::EXPORT)) {
        if (caps.have(tf::Fcap::NEEDS_FILE)) {
            text += "Read/write, needs original file for writing";
        } else {
            text = "Read/write";
        }
    } else if (caps.have(tf::Fcap::IMPORT)) {
        text = "Read";
    } else if (caps.have(tf::Fcap::EXPORT)) {
        if (caps.have(tf::Fcap::NEEDS_FILE)) {
            text += "Write, needs original file";
        } else {
            text = "Write";
        }
    } else {
        text = u8"—";
    }
    ui->lbCapsValue->setText(text);

    text = "<p>";
    str::append(text, proto->locDescription());
    if (ws.have(tf::Usfg::TEXT_ESCAPE)) {
        str::append(text,
            u8"<p>Line breaks are used by format itself. "
                  "To encode line breaks, UTranslator has several options:<br>"
                "• Disallow line breaks.<br>"
                "• Encode line breaks as in C-like languages: \\n&nbsp;=&nbsp;line&nbsp;break, \\\\&nbsp;=&nbsp;backslash.<br>"
                "• Encode line breaks with special string that’s disallowed in texts.");
    }
    ui->browAbout->setHtml(text);

    Super::exec();
}
