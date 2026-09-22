// My header
#include "QtDiff.h"

// Qt ex
#include "u_Qstrings.h"

// Portable diff
#include "PortableDiff.h"


qdif::FmtLib::FmtLib(const QTextCharFormat& x)
    : normal(x) {}


void qdif::writeChar(
        QTextCursor& cursor, char32_t text, const FmtLib& fmt)
{
    cursor.insertText(str::toQ(text), fmt.normal);
}


namespace {

    void writeSpan1(QTextCursor& cursor,
                    const char32_t* pStart, const char32_t* pEndPlus,
                    const QTextCharFormat& fmt)
    {
        if (pEndPlus == pStart)
            return;
        std::u32string_view span{ pStart, pEndPlus };
        cursor.insertText(str::toQ(span), fmt);
    }

}   // anon namespace


void qdif::writeSpan(
        QTextCursor& cursor, std::u32string_view text, const FmtLib& fmt)
{
    auto pStart = std::to_address(text.begin());
    auto pEnd = std::to_address(text.end());
    char buf[30];
    for (auto p = pStart; p != pEnd; ++p) {
        auto c = *p;
        switch (c) {
        // Control and format chars here that have special meaning in QTextBrowser
        case 0xFFFC: {  // Object replacement character — special meaning there
                writeSpan1(cursor, pStart, p, fmt.normal);
                snprintf(buf, std::size(buf), ":/Cp/%04X.png",
                         static_cast<int>(c));
                cursor.insertImage(buf);
                pStart = p + 1;
            } break;
        default: break;
        }
    }
    // remainder
    writeSpan1(cursor, pStart, pEnd, fmt.normal);
}


void qdif::write2(QTextCursor& cursor,
            std::u32string_view knownOrig,
            std::u32string_view orig,
            const QString& htSeparator)
{
    auto es = editScript(knownOrig, orig);

    QTextCharFormat fmtNormal = cursor.charFormat();
    FmtLib libNormal(fmtNormal);

    QTextCharFormat fmtAdd = fmtNormal;
    fmtAdd.setBackground(QColor{0xCC, 0xFF, 0xCC});
    FmtLib libAdd(fmtAdd);

    QTextCharFormat fmtDel = fmtNormal;
    fmtDel.setBackground(QColor{0xFF, 0xCC, 0xCC});
    FmtLib libDel(fmtDel);

    for (auto &v : es) {
        if (v.isCommon) {
            writeSpan(cursor, v.del, libNormal);
        } else if (!v.ins.empty()) {
            writeSpan(cursor, v.ins, libAdd);
        } else {
            cursor.insertImage(":/Diff/del.png");
        }
    }

    cursor.insertText("\n");
    cursor.insertHtml(htSeparator);
    cursor.insertText("\n");

    for (auto &v : es) {
        if (v.isCommon) {
            writeSpan(cursor, v.del, libNormal);
        } else if (!v.del.empty()) {
            writeSpan(cursor, v.del, libDel);
        } else {
            cursor.insertImage(":/Diff/ins.png");
        }
    }
}


void qdif::write1(QTextCursor& cursor,
            std::u32string_view orig)
{
    QTextCharFormat fmtNormal = cursor.charFormat();
    FmtLib libNormal(fmtNormal);
    for (auto c : orig) {
        writeChar(cursor, c, libNormal);
    }
}
