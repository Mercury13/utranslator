#include "FmMain.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

// Project-local
#include "d_Config.h"

template <class T>
inline T clampMin(const T& val, const T& min, const T& max)
    { return std::max(std::min(val, max), min); }

void setGeometry(QMainWindow& win, const QRect& rect)
{
    static constexpr int BUFFER_ZONE = 64;  // We should see ? px of window
    static constexpr int HEADER_ZONE = 24;  // We should see ? px of header

    auto screens = QApplication::screens();
    QRect desktopRect = screens[0]->availableVirtualGeometry();

    // Reduce width-height
    auto w = std::min(rect.width(), desktopRect.width());
    auto h = std::min(rect.height(), desktopRect.height());

    // Move x-y
    auto x = clampMin(rect.left(),
                      desktopRect.left() + BUFFER_ZONE - w,
                      desktopRect.right() - BUFFER_ZONE);
    auto y = clampMin(rect.top(),
                      desktopRect.top(),
                      desktopRect.bottom() - HEADER_ZONE);

    win.setGeometry(x, y, w, h);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FmMain w;

    // Load config
    {
        auto rect = w.geometry();
        config::init(rect);
        setGeometry(w, rect);
        if (config::window::isMaximized)
            w.setWindowState(w.windowState() | Qt::WindowMaximized);
    }

    w.show();
    bool r = a.exec();

    config::save(w.normalGeometry(), w.isMaximized());
    return r;
}
