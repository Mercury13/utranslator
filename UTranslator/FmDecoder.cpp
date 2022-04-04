#include "FmDecoder.h"
#include "ui_FmDecoder.h"

// Qt
#include <QDialogButtonBox>
#include <QPlainTextEdit>

// Qt ex
#include "QtConsts.h"

// Decoder
#include "Decoders.h"


///// MemoObject ///////////////////////////////////////////////////////////////


QString MemoObject::toQ()
{
    if (auto text = memo->textCursor().selectedText(); !text.isEmpty())
        return text;
    return memo->toPlainText();
}


void MemoObject::set(const QString& x)
{
    if (memo->textCursor().hasSelection()) {
        auto cursor = memo->textCursor();
        auto start = cursor.selectionStart();
        auto endOffset = memo->document()->characterCount() - cursor.selectionEnd();
        cursor.insertText(x);
        auto newEnd = memo->document()->characterCount() - endOffset;
        if (newEnd > start) {
            cursor.setPosition(start, QTextCursor::MoveAnchor);
            cursor.setPosition(newEnd, QTextCursor::KeepAnchor);
            memo->setTextCursor(cursor);
        }
    } else {
        memo->setPlainText(x);
    }
}


///// FmDecoder ////////////////////////////////////////////////////////////////

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


namespace {

    template <class Intermed, class Body>
    void runDecoder(StrObject& obj, const Body& body)
    {
        auto text = obj.get<Intermed>();
        auto sv = str::detail::toSv(text);
        auto result = body(sv);
        if (result != text)
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
{
    decodeMemo<std::u32string>(&decode::cpp);
    ui->lbDescription->setText("Decode C++ strings: \"first\\nsecond\" → first⤶second");
}
