#pragma once

#include <string>

namespace tr {

    enum class PrjType { ORIGINAL, FULL_TRANSL };

    struct PrjInfo {
        PrjType type = PrjType::ORIGINAL;
        struct Orig {
            std::string lang;
        } orig;
        struct Transl {
            std::string lang;
        } transl;
    };

}   // namespace tr
