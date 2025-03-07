#include "QstrObject.h"

// QT GUI
#include <QPlainTextEdit>


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
        // Thus we automatically got Undo
        memo->selectAll();
        memo->textCursor().insertText(x);
    }
}


