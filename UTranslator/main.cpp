#include "FmMain.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

// Qt misc
#include "RememberWindow.h"

// Project-local
#include "d_Config.h"

template <class T>
inline T clampMin(const T& val, const T& min, const T& max)
    { return std::max(std::min(val, max), min); }

int main(int argc, char *argv[])
{
    // For debugging HiDPI: glitchy, but better then nothing
    //qputenv("QT_SCALE_FACTOR", "1.25");

    QApplication a(argc, argv);
    FmMain w;

    // Load config
    {
        config::window::State state(w);
        config::init(state);
        config::window::setGeometry(w, state);
    }

    w.show();
    bool r = a.exec();

    config::save(config::window::State(w));
    return r;
}
