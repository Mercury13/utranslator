// My header
#include "DiffBrowser.h"

// C++
#include <iostream>

// Qt
#include <QTextCursor>
#include <QTextTableCell>


///// DiffMimeData /////////////////////////////////////////////////////////////


DiffMimeData::DiffMimeData(QTextCursor &aCursor) : fragment(aCursor) {}


QStringList DiffMimeData::formats() const
{
    if (!fragment.isEmpty()) {
        // have document (did not setup)
        return QStringList() << QString::fromLatin1("text/html")
                             << QString::fromLatin1("text/plain");
    } else {
        // have data (already setup)
        return Super::formats();
    }
}


QVariant DiffMimeData::retrieveData(const QString &mimeType, QMetaType type) const
{
    if (!fragment.isEmpty())
        setup();
    return Super::retrieveData(mimeType, type);
}


void DiffMimeData::setup() const
{
    This* that = const_cast<This*>(this);
    // HTML
    //std::cout << fragment.toHtml().toStdString() << std::endl;
    that->setData(QLatin1String("text/html"), fragment.toHtml().toUtf8());
    // Plain text
    that->setText(fragment.toPlainText());
    //std::cout << fragment.toPlainText().toStdString() << std::endl;
    fragment = QTextDocumentFragment();
}


///// DiffBrowser //////////////////////////////////////////////////////////////


QMimeData* DiffBrowser::createMimeDataFromSelection() const
{
    auto cursor = textCursor();
    return new DiffMimeData(cursor);
}
