// My header
#include "d_Config.h"

// Qt
#include <QCoreApplication>
#include <QStandardPaths>

// Qt ex
#include "RememberWindow.h"

// XML
#include "pugixml.hpp"


///// Vars /////////////////////////////////////////////////////////////////////

// progsets
progsets::DirMode progsets::dirMode = progsets::DirMode::INSTALLED;

// fname
std::filesystem::path fname::config;
std::filesystem::path fname::progsets;

// path
std::filesystem::path path::exeBundled;
std::filesystem::path path::exeAdmined;
std::filesystem::path path::config;

// config
hist::History config::history;


#define APP_NAME "UTranslator"

constexpr std::string_view APP_XML = APP_NAME ".xml";
constexpr std::string_view CONFIG_NAME = "config.xml";

namespace {

    void loadProgSets()
    {
        progsets::dirMode = progsets::DirMode::INSTALLED;

        if (fname::progsets.empty() || !std::filesystem::exists(fname::progsets))
            return;

        pugi::xml_document doc;
        doc.load_file(fname::progsets.c_str());
        auto root = doc.child("program");
        if (root.attribute("portable").as_bool(false))
            progsets::dirMode = progsets::DirMode::PORTABLE;
    }

    void loadConfig(config::window::State& state)
    {
        if (fname::config.empty() || !std::filesystem::exists(fname::config))
            return;

        pugi::xml_document doc;
        if (auto res = doc.load_file(fname::config.c_str())) {
            auto root = doc.child("config");

            // Window position
            config::window::load(root, state);

            auto tagHistory = root.child("history");
            config::history.load(tagHistory);
        }
    }

}   // anon namespace

void config::init(window::State& state)
{
    path::exeBundled = QCoreApplication::applicationFilePath().toStdWString();
#ifdef _WIN32
    path::exeBundled.remove_filename();
    path::exeAdmined = path::exeBundled;
    fname::progsets = path::exeAdmined / APP_XML;
#else
    #error Unknown OS
#endif

    loadProgSets();

    switch (progsets::dirMode) {
    case progsets::DirMode::INSTALLED: {
            std::filesystem::path localDir {
                QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).toStdWString() };
            path::config = localDir / APP_NAME;
        } break;
    case progsets::DirMode::PORTABLE:
        path::config = path::exeAdmined;
        break;
    }
    std::filesystem::create_directories(path::config);
    fname::config = path::config / CONFIG_NAME;
    loadConfig(state);
}


void config::save(const config::window::State& state)
{
    pugi::xml_document doc;

    auto root = doc.append_child("config");

    config::window::save(root, state);

    config::history.save(root, "history");
    doc.save_file(fname::config.c_str());
}
