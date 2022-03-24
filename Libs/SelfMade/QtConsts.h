#pragma once

#include <qnamespace.h>
#include <QKeySequence>

namespace QDlgType {
    Q_DECL_CONSTEXPR Qt::WindowFlags SIZEABLE =
            Qt::Dialog | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint | Qt::WindowCloseButtonHint
            | Qt::WindowMaximizeButtonHint;
    Q_DECL_CONSTEXPR Qt::WindowFlags FIXED =
            Qt::Dialog | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint;
    Q_DECL_CONSTEXPR Qt::WindowFlags UNCLOSEABLE =
            Qt::Dialog | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::MSWindowsFixedSizeDialogHint;
}


#define Q_YESNOCANCEL \
        (QMessageBox::StandardButtons( \
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))

// Hardcoded right now
constexpr auto Q_EDITKEY = Qt::Key_F2;

#define Q_IS_VERSION_ABOVE(major, minor) \
        (QT_VERSION_MAJOR > major  \
        || (QT_VERSION_MAJOR == major && QT_VERSION_MINOR >= minor))


inline QKeySequence qSeq(Qt::Modifier modifier, Qt::Key key)
    { return QKeySequence(static_cast<int>(key) | modifier); }
inline QKeySequence qSeq(Qt::Key key)
    { return QKeySequence(key); }
