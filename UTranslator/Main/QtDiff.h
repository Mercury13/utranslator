#pragma once

#include <QTextCursor>

namespace qdif {
    struct FmtLib {
        QTextCharFormat normal;

        FmtLib(const QTextCharFormat& x);
    };

    void writeChar(
            QTextCursor& cursor, char32_t text,
            const FmtLib& fmt);

    void writeSpan(
            QTextCursor& cursor, std::u32string_view text,
            const FmtLib& fmt);

    void write2(
            QTextCursor& cursor,
            std::u32string_view knownOrig,
            std::u32string_view orig,
            const QString& htSeparator);

    void write1(QTextCursor& cursor, std::u32string_view text);

}
