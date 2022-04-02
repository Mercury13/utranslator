#pragma once

#include <string>

namespace tr {

    enum class PrjType { ORIGINAL, FULL_TRANSL };
    constexpr int PrjType_N = static_cast<int>(PrjType::FULL_TRANSL) + 1;
    extern const char* prjTypeNames[PrjType_N];

    struct PrjInfo {
        PrjType type = PrjType::ORIGINAL;
        struct Orig {
            std::string lang;
            bool isIdless = false;
        } orig;
        struct Transl {
            std::string lang;
        } transl;
    };

}   // namespace tr
