#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class FmNew;

namespace tr {
    class Project;
}

template <class T>
class Uptr : public std::unique_ptr<T>
{
    using Super = std::unique_ptr<T>;
public:
    using Super::Super;
    using Super::operator =;

    template <class... UU>
    T& ensure(UU&&... u) {
        if (!*this) {
            *this = std::make_unique<T>(std::forward<UU>(u)...);
        }
        return **this;
    }
};

class FmMain : public QMainWindow
{
    Q_OBJECT

public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

    void doNew();

private:
    Ui::FmMain *ui;

    std::shared_ptr<tr::Project> project;

    Uptr<FmNew> fmNew;
};
