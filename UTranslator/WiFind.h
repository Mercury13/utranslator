#ifndef WIFIND_H
#define WIFIND_H

#include <QWidget>

namespace Ui {
class WiFind;
}

class WiFind : public QWidget
{
    Q_OBJECT

public:
    explicit WiFind(QWidget *parent = nullptr);
    ~WiFind();

private:
    Ui::WiFind *ui;
};

#endif // WIFIND_H
