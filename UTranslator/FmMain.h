#pragma once

// Qt
#include <QMainWindow>
#include <QAbstractItemModel>

// Project
// No one uses FmMain → you can import EVERYTHING
#include "TrProject.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class FmNew;

class PrjTreeModel final : public QAbstractItemModel
{
private:
    using Super = QAbstractItemModel;
public:
    tr::UiObject* toObj(const QModelIndex& index) const;
    QModelIndex toIndex(const tr::UiObject& obj, int col) const;
    QModelIndex toIndex(const tr::UiObject* p, int col) const;
    QModelIndex toIndex(const std::shared_ptr<tr::UiObject>& p, int col) const
            { return toIndex(p.get(), col); }
    QModelIndex index(int row, int column,
                const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setProject(std::shared_ptr<tr::Project> aProject);
private:
    static constexpr int DUMMY_COL = 0;
    std::shared_ptr<tr::Project> project;   // will hold old project
};

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
    using Super = QMainWindow;
    using This = FmMain;
public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

public slots:
    void doNew();

private:
    Ui::FmMain *ui;

    PrjTreeModel treeModel;
    std::shared_ptr<tr::Project> project;

    Uptr<FmNew> fmNew;

    void adaptLayout();
};
