// My header
#include "QtDiff.h"

// Qt ex
#include "u_Qstrings.h"
#include "u_Array.h"


qdif::SimpleSplit qdif::simpleSplit(std::u32string_view a, std::u32string_view b)
{
    size_t minLen = std::min(a.length(), b.length());

    size_t commonPrefLen = 0;
    while (commonPrefLen < minLen
           && a[commonPrefLen] == b[commonPrefLen]) {
        ++commonPrefLen;
    }

    minLen -= commonPrefLen;
    const size_t lenA1 = a.length() - 1;
    const size_t lenB1 = b.length() - 1;
    size_t commonSuffLen = 0;
    while (commonSuffLen < minLen
           && a[lenA1 - commonSuffLen] == b[lenB1 - commonSuffLen]) {
        ++commonSuffLen;
    }

    const size_t suffA = a.length() - commonSuffLen;
    const size_t suffB = b.length() - commonSuffLen;
    return {
        .commonPrefix = a.substr(0, commonPrefLen),
        .aMid = a.substr(commonPrefLen, suffA - commonPrefLen),
        .bMid = b.substr(commonPrefLen, suffB - commonPrefLen),
        .commonSuffix = a.substr(suffA),
    };
}


namespace {

    constexpr std::u32string_view UEMPTY {};

    enum class Dir : unsigned char { COM, DEL, CHG, INS };

    qdif::Pair& backOf(qdif::EditScript& r, bool needCommon,
                       const char32_t& a, const char32_t& b)
    {
        if (!r.empty()) {
            auto& bk = r.back();
            if (bk.isCommon == needCommon)
                return bk;
        }
        auto& bk1 = r.emplace_back();
        bk1.del = std::u32string_view { &a, 0 };
        bk1.ins = std::u32string_view { &b, 0 };
        bk1.isCommon = needCommon;
        return bk1;
    }

    void inc1(std::u32string_view& subst, size_t& index)
    {
        subst = std::u32string_view { subst.data(), subst.size() + 1 };
        ++index;
    }

    void appendEditScript(
            qdif::EditScript& r,
            std::u32string_view a,
            std::u32string_view b)
    {
        if (a.empty()) {
            if (b.empty()) {
                return;
            } else {
                r.emplace_back( UEMPTY, b );
                return;
            }
        } else if (b.empty()) {
            r.emplace_back( UEMPTY, a );
            return;
        }

        // cumulative
        Array2d<size_t> cm(a.length() + 1, b.length() + 1);
        // direction
        Array2d<Dir> dr(a.length() + 1, b.length() + 1);

        // Fill edge things
        for (size_t i = 0; i <= a.length(); ++i) {
            cm(i, b.length()) = a.length() - i;
            dr(i, b.length()) = Dir::DEL;
        }
        for (size_t j = 0; j <= b.length(); ++j) {
            cm(a.length(), j) = b.length() - j;
            dr(a.length(), j) = Dir::INS;
        }

        // Go by matrix!
        for (size_t i = a.length(); i != 0;) { --i;
            for (size_t j = b.length(); j != 0;) { --j;
                if (a[i] == b[j]) {
                    cm(i, j) = cm(i + 1, j + 1);
                    dr(i, j) = Dir::COM;
                } else {
                    auto& cij = cm(i, j);
                    auto& dij = dr(i, j);
                    cij = cm(i + 1, j + 1) + 1;
                    dij = Dir::CHG;
                    auto distDel = cm(i + 1, j) + 1;
                    if (distDel < cij) {
                        cij = distDel;
                        dij = Dir::DEL;
                    }
                    auto distIns = cm(i, j + 1) + 1;
                    if (distIns < cij) {
                        cij = distIns;
                        dij = Dir::INS;
                    }
                }
            }
        }

        std::cout << "Cumulative (0, 0) = " << cm(0, 0) << std::endl;

        // Forward move
        size_t ii = 0, jj = 0;
        while (ii != a.length() || jj != b.length()) {
            switch (dr(ii, jj)) {
            case Dir::COM: {
                    std::cout << "Common at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, true, a[ii], b[jj]);
                    inc1(bk.del, ii);
                    ++jj;
                } break;
            case Dir::CHG: {
                    std::cout << "Change at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, a[ii], b[jj]);
                    inc1(bk.del, ii);
                    inc1(bk.ins, jj);
                } break;
            case Dir::DEL: {
                    std::cout << "Delete at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, a[ii], b[jj]);
                    inc1(bk.del, ii);
                } break;
            case Dir::INS: {
                    std::cout << "Insert at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, a[ii], b[jj]);
                    inc1(bk.ins, jj);
                } break;
            }
        }
    }

}


qdif::EditScript qdif::editScript(std::u32string_view a, std::u32string_view b)
{
    EditScript r;
    // For simplicity, we cut head and tail
    auto split = simpleSplit(a, b);
    if (!split.commonPrefix.empty())
        r.emplace_back(split.commonPrefix, UEMPTY, true);
    appendEditScript(r, split.aMid, split.bMid);
    if (!split.commonSuffix.empty())
        r.emplace_back(split.commonSuffix, UEMPTY, true);
    return r;
}


qdif::FmtLib::FmtLib(const QTextCharFormat& x)
    : normal(x) {}


void qdif::writeChar(
        QTextCursor& cursor, char32_t text, const FmtLib& fmt)
{
    cursor.insertText(str::toQ(text), fmt.normal);
}


void qdif::writeSpan(
        QTextCursor& cursor, std::u32string_view text, const FmtLib& fmt)
{
    cursor.insertText(str::toQ(text), fmt.normal);
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
