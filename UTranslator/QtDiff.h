#pragma once

#include <QTextCursor>

#include "u_Vector.h"

namespace qdif {

    struct Pair {
        std::u32string_view del, ins;
        bool isCommon = false;
    };
    struct SimpleSplit {
        std::u32string_view commonPrefix, aMid, bMid, commonSuffix;
    };

    using EditScript = SafeVector<Pair>;

    /// Gets common prefix and suffix, to reduce O(n²) of edit distance algo
    SimpleSplit simpleSplit(std::u32string_view a, std::u32string_view b);

    /// Gets edit script which turns a to b
    /// Script consists of Pair’s whose isCommon interleaves true/false.
    EditScript editScript(std::u32string_view a, std::u32string_view b);

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
