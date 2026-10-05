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
            tr::Project& project,
            const pugi::xml_document& doc,
            const std::filesystem::path& basePath);
    void load(
            tr::Project& project,
            const std::filesystem::path& aFname);

    enum class DetectedFormat : unsigned char {
        SAVE, XLIFF };
    DetectedFormat smartLoad(
            tr::Project& project,
            const std::filesystem::path& aFname);

    tr::UpdateInfo updateData(
            tr::Project& project, tr::TrashMode trashMode);
    void updateReference(tr::Project& project);

}   // namespace sav
