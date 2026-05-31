// My header
#include "PortableDiff.h"

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
    constexpr size_t W_CHG = 10;        // Weight of change
    constexpr size_t W_DEL = 10;        // Weight of deletion
    constexpr size_t W_INS = W_DEL;     // Weight of insertion
    constexpr size_t W_BONUS = 1;       // A small bonus for several identical commands in a row
    constexpr size_t W_DEL_BONUS = W_DEL - W_BONUS;
    constexpr size_t W_INS_BONUS = W_DEL_BONUS;
    constexpr size_t SZ_SMALL_CHG = 1;  // size of common span <= X → stick to del/ins

    enum class Dir : unsigned char { COM, DEL, CHG, INS };

    [[maybe_unused]] void appendN(std::u32string_view& subst, size_t n)
    { subst = std::u32string_view { subst.data(), subst.size() + n }; }

    qdif::Pair& backOf(qdif::EditScript& r, bool needCommon,
                       const char32_t* a, const char32_t* b)
    {
        if (!r.empty()) {
            auto& bk = r.back();
            if (bk.isCommon == needCommon)
                return bk;
            // Stick small common spans to prev del/ins
            // There WILL be a few common letters → don’t make noise
            if constexpr (SZ_SMALL_CHG != 0) {
                if (r.size() >= 2 && bk.isCommon && bk.del.size() <= SZ_SMALL_CHG) {
                    // bk1 += bk
                    auto& bk1 = ((&bk)[-1]);
                    appendN(bk1.del, bk.del.size());
                    appendN(bk1.ins, bk.del.size());

                    // Pop, check once again
                    // See bug 1 description below
                    r.pop_back();
                    // pop_back guarantees to preserve iterators/pointers
                    // except bk and end, so OK!
                    if (bk1.isCommon == needCommon)
                        return bk1;
                }
            }
        }
        auto& newBk = r.emplace_back();
        newBk.del = std::u32string_view { a, 0 };
        newBk.ins = std::u32string_view { b, 0 };
        newBk.isCommon = needCommon;
        return newBk;
    }

    // Bug 1: медного → бронзового
    //    (Russian genitive: made of copper → made of bronze)
    //   Final “ого” is common suffix
    //   C = changed, U = common, I = inserted
    //   CCCU → CCCUIII
    // 0 = common prefix
    // 1 = three changed
    // 2 = one common
    // (no number) three inserted? — we see one common, stick with three changed
    //     and start writing those inserted at 2
    // so BAD: 1 = four changed, 2 = three inserted, 3 = common suffix
    //   so we see both deleted text “медн” and insert sign
    // RIGHT: stick 1+2, check once again and write at 1!!!!!
    //   0 = common prefix, 1 = changed 4→7, 2 = common suffix

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
            r.emplace_back( a, UEMPTY );
            return;
        }

        // cumulative
        Array2d<size_t> cm(a.length() + 1, b.length() + 1);
        // direction
        Array2d<Dir> dr(a.length() + 1, b.length() + 1);

        // Fill edge things
        for (size_t i = 0; i <= a.length(); ++i) {
            cm(i, b.length()) = (a.length() - i) * W_DEL_BONUS;
            dr(i, b.length()) = Dir::DEL;
        }
        for (size_t j = 0; j <= b.length(); ++j) {
            cm(a.length(), j) = (b.length() - j) * W_INS_BONUS;
            dr(a.length(), j) = Dir::INS;
        }
        dr(a.length(), b.length()) = Dir::COM;

        // Go by matrix!
        for (size_t i = a.length(); i != 0;) { --i;
            for (size_t j = b.length(); j != 0;) { --j;
                if (a[i] == b[j]) {
                    cm(i, j) = cm(i + 1, j + 1);
                    dr(i, j) = Dir::COM;
                } else {
                    auto& cij = cm(i, j);
                    auto& dij = dr(i, j);
                    // Change
                    cij = cm(i + 1, j + 1) + W_CHG;
                    if constexpr (W_BONUS != 0) {
                        if (dr(i + 1, j + 1) == Dir::CHG)
                            cij -= W_BONUS;
                    }
                    dij = Dir::CHG;
                    // Delete
                    auto distDel = cm(i + 1, j) + W_DEL;
                    if constexpr (W_BONUS != 0) {
                        if (dr(i + 1, j) == Dir::DEL)
                            distDel -= W_BONUS;
                    }
                    if (distDel <= cij) {
                        cij = distDel;
                        dij = Dir::DEL;
                    }
                    // Insert
                    auto distIns = cm(i, j + 1) + W_INS;
                    if constexpr (W_BONUS != 0) {
                        if (dr(i, j + 1) == Dir::INS)
                            distIns -= W_BONUS;
                    }
                    if (distIns <= cij) {
                        cij = distIns;
                        dij = Dir::INS;
                    }
                }
            }   // for j
        }   // for i

        //std::cout << "Cumulative (0, 0) = " << cm(0, 0) << std::endl;

        // Forward move
        size_t ii = 0, jj = 0;
        while (ii != a.length() || jj != b.length()) {
            if (ii > a.length() || jj > b.length()) {
                throw std::logic_error("Went too far away");
            }
            const char32_t* pA = a.data() + ii;
            const char32_t* pB = b.data() + jj;
            switch (dr(ii, jj)) {
            case Dir::COM: {
                    //std::cout << "Common at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, true, pA, pB);
                    inc1(bk.del, ii);
                    inc1(bk.ins, jj);
                } break;
            case Dir::CHG: {
                    //std::cout << "Change at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, pA, pB);
                    inc1(bk.del, ii);
                    inc1(bk.ins, jj);
                } break;
            case Dir::DEL: {
                    //std::cout << "Delete at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, pA, pB);
                    inc1(bk.del, ii);
                } break;
            case Dir::INS: {
                    //std::cout << "Insert at " << ii << "/" << jj << std::endl;
                    auto& bk = backOf(r, false, pA, pB);
                    inc1(bk.ins, jj);
                } break;
            }
        }   // while ii, jj
    }   // appendEditScript

}   // anon namespace


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
