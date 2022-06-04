#pragma once

#include <QTextBrowser>
#include <QMimeData>
#include <QTextDocumentFragment>


class DiffMimeData : public QMimeData
{
private:
    using Super = QMimeData;
    using This = DiffMimeData;
public:
    DiffMimeData(QTextCursor &aCursor);
    QStringList formats() const override;
protected:
    QVariant retrieveData(const QString &mimeType, QMetaType type) const override;
private:
    void setup() const;

    mutable QTextDocumentFragment fragment;
};


class DiffBrowser : public QTextBrowser
{
private:
    using Super = QTextBrowser;
public:
    using Super::Super;
protected:
    QMimeData* createMimeDataFromSelection() const override;
};
