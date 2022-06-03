// My header
#include "DiffBrowser.h"


///// DiffMimeData /////////////////////////////////////////////////////////////


QStringList DiffMimeData::formats() const
{
    if (!fragment.isEmpty()) {
        return QStringList() << QString::fromLatin1("text/plain") << QString::fromLatin1("text/html");
    } else {
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
    that->setData(QLatin1String("text/html"), fragment.toHtml().toUtf8());
    // Plain text
    that->setText(fragment.toPlainText());
    fragment = QTextDocumentFragment();
}


///// DiffBrowser //////////////////////////////////////////////////////////////


QMimeData* DiffBrowser::createMimeDataFromSelection() const
{
    auto cursor = textCursor();
    QTextDocumentFragment fragment(cursor);
    return new DiffMimeData(fragment);
}
