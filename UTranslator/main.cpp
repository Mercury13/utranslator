#include "FmMain.h"

#include <QApplication>

// Project-local
#include "d_Config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FmMain w;

    // Load config
    {
        auto rect = w.geometry();
        config::init(rect);
        w.setGeometry(rect);
        if (config::window::isMaximized)
            w.setWindowState(w.windowState() | Qt::WindowMaximized);
    }

    w.show();
    bool r = a.exec();

    config::save(w.normalGeometry(), w.isMaximized());
    return r;
}
