// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Libs
#include "u_Qstrings.h"

// UI forms
#include "FmNew.h"

///// PrjTreeModel /////////////////////////////////////////////////////////////

namespace {
    enum {
        COL_ID = 0,
        COL_ORIG = 1,
        COL_TRANSL = 2,
        N_ORIG = COL_ORIG + 1,
        N_TRANSL = COL_TRANSL + 1,
    };
}   // anon namespace

void PrjTreeModel::setProject(std::shared_ptr<tr::Project> aProject)
{
    beginResetModel();
    project = aProject;
    endResetModel();
}

tr::UiObject* PrjTreeModel::toObj(const QModelIndex& index) const
{
    auto ptr = index.internalPointer();
    if (!ptr) {
        if (project)
            project->checkCanary();
        return project.get();
    }
    auto r = static_cast<tr::UiObject*>(ptr);
    r->checkCanary();
    return r;
}

QModelIndex PrjTreeModel::toIndex(const tr::UiObject& obj, int col) const
{
    if (obj.type() == tr::ObjType::PROJECT)
        return {};
    return createIndex(obj.cache.index, col, &obj);
}

QModelIndex PrjTreeModel::toIndex(const tr::UiObject* p, int col) const
{
    if (!p)
        return {};
    return toIndex(*p, col);
}

QModelIndex PrjTreeModel::index(int row, int col,
            const QModelIndex &parent) const
{
    auto obj = toObj(parent);
    auto child = obj->child(row);
    return toIndex(child, col);
}

QModelIndex PrjTreeModel::parent(const QModelIndex &child) const
{
    auto obj = toObj(child);
    return toIndex(obj->parent(), DUMMY_COL);
}

int PrjTreeModel::rowCount(const QModelIndex &parent) const
{
    auto obj = toObj(parent);
    return obj->nChildren();
}

int PrjTreeModel::columnCount(const QModelIndex &) const
{
    if (!project)
        return N_ORIG;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        return N_ORIG;
    case tr::PrjType::FULL_TRANSL:
        return N_TRANSL;
    }
    return N_TRANSL;
}

QVariant PrjTreeModel::data(const QModelIndex &index, int role) const
{
    if (!project)
        return {};
    switch (role) {
    case Qt::DisplayRole: {
            auto obj = toObj(index);
            switch (index.column()) {
            case COL_ID:
                return str::toQ(obj->idColumn());
            case COL_ORIG:
                return str::toQ(obj->origColumn());
            case COL_TRANSL:
                return str::toQ(obj->translColumn());
            default:
                return {};
            }
        }
    default:
        return {};
    }
}

///// FmMain ///////////////////////////////////////////////////////////////////

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);

    // Setup layout
    ui->splitMain->setSizes({1, 1});

    // Stack
    ui->stackMain->setCurrentWidget(ui->pageStart);

    // Model
    ui->treeStrings->setModel(&treeModel);

    // Signals/slots
    connect(ui->acNew, &QAction::triggered, this, &This::doNew);
    connect(ui->btStartNew, &QPushButton::clicked, this, &This::doNew);
}

FmMain::~FmMain()
{
    delete ui;
}

void FmMain::doNew()
{
    if (auto result = fmNew.ensure(this).exec(0)) {
        project = std::make_shared<tr::Project>(std::move(*result));
        project->addTestOriginal();
        treeModel.setProject(project);
        ui->stackMain->setCurrentWidget(ui->pageMain);
    }
}
