#include "FmDecoder.h"
#include "ui_FmDecoder.h"

// Qt
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QShortcut>

// Qt ex
#include "QtConsts.h"

// Libs
#include "u_Strings.h"
#include "QstrObject.h"

// Decoder
#include "u_Decoders.h"


///// FmDecoder ////////////////////////////////////////////////////////////////

FmDecoder::FmDecoder(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED | Qt::WindowContextHelpButtonHint),
    ui(new Ui::FmDecoder)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);

    // Different decode algos
    connect(ui->btDecodeC, &QPushButton::clicked, this, &This::decodeCpp);
    connect(ui->btDecodeBr, &QPushButton::clicked, this, &This::decodeBr);

    ui->memo->setWhatsThis(
              u8"Type or paste text here.<br>"
                "To revert decoding use Undo (Ctrl+Z).");
    ui->btDecodeC->setWhatsThis(
              u8"Decode strings pasted from C++ source<br>"
                "Before: <b>u8\"first\\n\" \"\\\\second\"sv,</b><br>"
                "After: <b>first<br>\\second</b>");
    ui->btDecodeBr->setWhatsThis(
              u8"It’s hard to do linebreak-related wikitext things "
                  "right in programming language source. "
                  "Such wikitext contains &lt;br&gt; and &lt;p&gt; tags.<br>"
                "<b>&lt;br&gt;</b> → <b>&lt;br&gt;⤶</b><br>"
                "<b>&lt;p&gt;</b> → <b>⤶⤶</b>");

    // Shortcut v1 — Ctrl+Enter
    QShortcut* shcut = new QShortcut(Qt::CTRL | Qt::Key_Enter, this);
    connect(shcut, &QShortcut::activated, this, &This::accept);
    // Shortcut v2 — Ctrl+Return
    shcut = new QShortcut(Qt::CTRL | Qt::Key_Return, this);
    connect(shcut, &QShortcut::activated, this, &This::accept);
}

FmDecoder::~FmDecoder()
{
    delete ui;
}


bool FmDecoder::exec(QstrObject* obj)
{
    if (obj) {
        ui->memo->setPlainText(obj->toQ());
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(obj);
    ui->memo->setFocus();
    auto q = Super::exec();
    if (q && obj)
        obj->set(ui->memo->toPlainText());
    return q;
}


namespace {

    template <class Intermed, class Body>
    void runDecoder(StrObject& obj, const Body& body)
    {
        auto text = obj.get<Intermed>();
        auto sv = str::detail::toSv(text);
        auto result = body(sv);
        if (!result.empty() && result != text)
            obj.set(result);
    }

}   // anon namespace


template <class Intermed, class Body>
void FmDecoder::decodeMemo(const Body& body)
{
    MemoObject mo(ui->memo);
    runDecoder<Intermed>(mo, body);
}

void FmDecoder::decodeCpp()
    { decodeMemo<std::u32string>(&decode::cpp); }

void FmDecoder::decodeBr()
    { decodeMemo<std::wstring>(&decode::htmlBrP); }
