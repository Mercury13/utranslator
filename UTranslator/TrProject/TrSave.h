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

    void load(
            const pugi::xml_document& doc,
            const std::filesystem::path& basePath);
    void load(const std::filesystem::path& aFname);

    tr::UpdateInfo updateData(
            tr::Project& project, tr::TrashMode trashMode);

}   // namespace sav
