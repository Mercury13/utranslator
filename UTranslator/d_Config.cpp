// My header
#include "d_Config.h"

// Qt
#include <QCoreApplication>
#include <QStandardPaths>

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
bool config::window::isMaximized;
QSize config::window::desktopSize { 0, 0 };


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

    void loadConfig(QRect& winRect)
    {
        if (fname::config.empty() || !std::filesystem::exists(fname::config))
            return;

        pugi::xml_document doc;
        if (auto res = doc.load_file(fname::config.c_str())) {
            auto root = doc.child("config");
            auto oldW = winRect.width();
            auto oldH = winRect.height();
            auto hWin = root.child("window");
                winRect.setLeft(hWin.attribute("x").as_int(winRect.left()));
                winRect.setTop(hWin.attribute("y").as_int(winRect.top()));
                winRect.setWidth(hWin.attribute("w").as_int(oldW));
                winRect.setHeight(hWin.attribute("h").as_int(oldH));
                config::window::isMaximized = hWin.attribute("max").as_bool();
                auto hDesktop = hWin.child("desktop");
                    config::window::desktopSize.setWidth (hDesktop.attribute("w").as_int());
                    config::window::desktopSize.setHeight(hDesktop.attribute("h").as_int());

            auto tagHistory = root.child("history");
            config::history.load(tagHistory);
        }
    }

}   // anon namespace

void config::init(QRect& winRect)
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
    loadConfig(winRect);
}


void config::save(
        const QRect& winRect,
        bool isMaximized)
{
    pugi::xml_document doc;

    auto root = doc.append_child("config");

    if (!window::desktopSize.isEmpty()) {
        auto hWin = root.append_child("window");
            hWin.append_attribute("x") = winRect.left();
            hWin.append_attribute("y") = winRect.top();
            hWin.append_attribute("w") = winRect.width();
            hWin.append_attribute("h") = winRect.height();
            hWin.append_attribute("max") = isMaximized;

        auto hDesk = hWin.append_child("desktop");
            hDesk.append_attribute("w") = window::desktopSize.width();
            hDesk.append_attribute("h") = window::desktopSize.height();
    }

    config::history.save(root, "history");
    doc.save_file(fname::config.c_str());
}
