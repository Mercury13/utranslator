#pragma once

#include <string_view>
#include <vector>

namespace qdif {

    struct Pair {
        std::u32string_view del, ins;
        bool isCommon = false;
    };
    struct SimpleSplit {
        std::u32string_view commonPrefix, aMid, bMid, commonSuffix;
    };

    using EditScript = std::vector<Pair>;

    /// Gets common prefix and suffix, to reduce O(n²) of edit distance algo
    SimpleSplit simpleSplit(std::u32string_view a, std::u32string_view b);

    /// Gets edit script which turns a to b
    /// Script consists of Pair’s whose isCommon interleaves true/false.
    EditScript editScript(std::u32string_view a, std::u32string_view b);

}
