// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Qt
#include <QItemSelectionModel>

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

tr::UiObject* PrjTreeModel::toObjOr(
        const QModelIndex& index, tr::UiObject* dflt) const
{
    auto ptr = index.internalPointer();
    if (!ptr)
        return dflt;
    auto r = static_cast<tr::UiObject*>(ptr);
    r->checkCanary();
    if (r->objType() == tr::ObjType::PROJECT)
        return dflt;
    return r;
}

tr::UiObject* PrjTreeModel::toObj(const QModelIndex& index) const
{
    auto r = toObjOr(index, project.get());
    if (!r)
        throw std::logic_error("[toObj] Somehow got nullptr");
    return r;
}


QModelIndex PrjTreeModel::toIndex(const tr::UiObject& obj, int col) const
{
    if (obj.objType() == tr::ObjType::PROJECT)
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

namespace {
    constexpr QColor BG_MODIFIED { 0xFA, 0xF0, 0xE6 };
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
    case Qt::BackgroundRole: {
            auto obj = toObj(index);
            switch (index.column()) {
            case COL_ID:
                if (obj->cache.mod.id)
                    return BG_MODIFIED;
                return {};
            case COL_ORIG:
                if (obj->cache.mod.original)
                    return BG_MODIFIED;
                return {};
            case COL_TRANSL:
                if (obj->cache.mod.translation)
                    return BG_MODIFIED;
                return {};
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

    // Splitter
    auto h = height();
    ui->splitMain->setSizes({ h, 1 });
    ui->splitMain->setStretchFactor(0, 2);
    ui->splitMain->setStretchFactor(1, 1);

    // Stack
    ui->stackMain->setCurrentWidget(ui->pageStart);

    // Helps
    ui->wiOriginalHelp->hide();

    // Model
    ui->treeStrings->setModel(&treeModel);

    // Signals/slots: tree etc.
    connect(ui->treeStrings->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &This::treeCurrentChanged);

    // Signals/slots: menu
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
        project->doShare(project);
        project->addTestOriginal();
        treeModel.setProject(project);
        ui->stackMain->setCurrentWidget(ui->pageMain);
        ui->treeStrings->setFocus(Qt::FocusReason::OtherFocusReason);
        adaptLayout();
    }
}


void FmMain::adaptLayout()
{
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        ui->wiId->show();
        ui->grpTranslation->hide();
        ui->menuOriginal->setEnabled(true);
        break;
    case tr::PrjType::FULL_TRANSL:
        ui->wiId->hide();
        ui->grpTranslation->show();
        ui->menuOriginal->setEnabled(false);
        break;
    }
}


void FmMain::treeCurrentChanged(
        const QModelIndex& current, const QModelIndex& former)
{
    auto formerObj = treeModel.toObj(former);
    auto currentObj = treeModel.toObj(current);

    if (auto currentPrj = currentObj->project()) {
        if (formerObj->project() == currentPrj)
            saveObject(*formerObj);
        loadObject(*currentObj);
    }
}


void FmMain::loadObject(tr::UiObject& obj)
{
    ui->edId->setText(str::toQ(obj.idColumn()));
    // Original/translation
    if (auto tr = obj.translatable()) {
        ui->grpOriginal->setEnabled(true);
        ui->grpTranslation->setEnabled(true);
        ui->memoOriginal->setPlainText(str::toQ(tr->original));
        ui->memoTranslation->setPlainText(str::toQ(tr->translationSv()));
    } else {
        ui->grpOriginal->setEnabled(false);
        ui->grpTranslation->setEnabled(false);
        ui->memoOriginal->clear();
        ui->memoTranslation->clear();
    }
    // Comment
    if (auto comm = obj.comments()) {
        ui->grpComment->setEnabled(true);
        if (project->info.type == tr::PrjType::ORIGINAL) {  // original
            ui->memoComment->setPlainText(str::toQ(comm->authors));
        } else {    // both full and patch translation
            ui->memoComment->setPlainText(str::toQ(comm->translators));
        }
    } else {
        ui->grpComment->setEnabled(false);
        ui->memoComment->clear();
    }
}


namespace {

    ///  Turns CR and CR+LF to LF
    void normalizeEol(QString& x)
    {
        if (x.indexOf('\r') >= 0) {
            x.replace("\r\n", "\n");
            x.replace('\r', '\n');
        }
    }

    std::u8string_view toText(QString x, std::string& cache)
    {
        normalizeEol(x);
        cache = x.toStdString();
        return { reinterpret_cast<const char8_t*>(cache.data()), cache.length() };
    }

}   // anon namespace


void FmMain::saveObject(tr::UiObject& obj)
{
    /// @todo [urgent] saveObject
    //auto tr = obj.translatable();
    //auto comm = obj.comments();
    std::string cache;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        obj.setId(str::toU8(ui->edId->text(), cache), tr::Modify::YES);
        //if (tr)
        //    tr->original = toStorage(ui->memo)
        break;
    case tr::PrjType::FULL_TRANSL:
        break;
    }
}


void FmMain::saveCurrObject()
{
    auto index = ui->treeStrings->currentIndex();
    auto obj = treeModel.toObj(index);
    saveObject(*obj);
}
