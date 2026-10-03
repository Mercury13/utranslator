#pragma once

#include "TrProject.h"

namespace sav {

    void save(tr::Project& project);
    void save(
            tr::Project& project,
            const std::filesystem::path& fname);
    void saveCopy(
            const tr::Project& project,
            const std::filesystem::path& fname);

}   // namespace sav
