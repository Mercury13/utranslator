// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Qt
#include <QItemSelectionModel>
#include <QMessageBox>

// Libs
#include "u_Qstrings.h"

// UI forms
#include "FmNew.h"
#include "FmDisambigPair.h"


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


QVariant PrjTreeModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    static constinit const char* colNames[] {
        "ID", "Original", "Translation" };

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (static_cast<size_t>(section) < std::size(colNames)) {
                return colNames[section];
            }
        }
    }
    return {};
}



Thing<tr::File> PrjTreeModel::addHostedFile()
{
    auto newId = project->makeId(u8"file", u8".txt");
    auto newIndex = project->nChildren();
    beginInsertRows(QModelIndex(), newIndex, newIndex);
    auto file = project->addFile(newId, tr::Modify::YES);
    endInsertRows();
    return { file, toIndex(file, 0) };
}


Thing<tr::Group> PrjTreeModel::addHostedGroup(
        const std::shared_ptr<tr::VirtualGroup>& parent)
{
    if (!parent)
        return {};
    auto newId = parent->makeId(u8"g", {});
    auto newIndex = parent->nChildren();
    beginInsertRows(toIndex(parent, 0), newIndex, newIndex);
    auto group = parent->addGroup(newId, tr::Modify::YES);
    endInsertRows();
    return { group, toIndex(group, 0) };
}


Thing<tr::Text> PrjTreeModel::addText(
        const std::shared_ptr<tr::VirtualGroup>& parent)
{
    if (!parent)
        return {};
    auto newId = parent->makeId({}, {});
    auto newIndex = parent->nChildren();
    beginInsertRows(toIndex(parent, 0), newIndex, newIndex);
    auto text = parent->addText(newId, {}, tr::Modify::YES);
    endInsertRows();
    return { text, toIndex(text, 0) };
}


std::shared_ptr<tr::Entity> PrjTreeModel::extract(tr::UiObject* obj)
{
    if (!obj)
        return {};
    // Get parent/check once again
    auto pnt = obj->parent();
    auto pntIndex = toIndex(pnt, 0);
    auto myIndex = obj->cache.index;
    auto that1 = index(myIndex, 0, pntIndex);
    auto obj1 = toObj(that1);
    if (obj != obj1)
        throw std::logic_error("[PrjTreeModel.doDelete] Check failed!");
    beginRemoveRows(pntIndex, myIndex, myIndex);
    auto r = toObj(pntIndex)->extractChild(myIndex);
    endRemoveRows();
    if (!r)
        throw std::logic_error("[PrjTreeModel.doDelete] Identified as deletable, but did not delete");
    return r;
}


void PrjTreeModel::paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
{
    constexpr auto MASK_SF = QStyle::State_Selected | QStyle::State_HasFocus;
    if ((option.state & MASK_SF) == MASK_SF) {  // Sel+focus → leave only sel
        QStyleOptionViewItem option1 = option;
        option1.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, option1, index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
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
    ui->treeStrings->setItemDelegate(&treeModel);

    // Signals/slots: tree etc.
    connect(ui->treeStrings->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &This::treeCurrentChanged);

    // Signals/slots: menu
    // File
    connect(ui->acNew, &QAction::triggered, this, &This::doNew);
    connect(ui->btStartNew, &QPushButton::clicked, this, &This::doNew);
    // Go
    connect(ui->acGoBack, &QAction::triggered, this, &This::goBack);
    connect(ui->acGoNext, &QAction::triggered, this, &This::goNext);
    // Original
    connect(ui->acAddHostedFile, &QAction::triggered, this, &This::addHostedFile);
    connect(ui->acAddHostedGroup, &QAction::triggered, this, &This::addHostedGroup);
    connect(ui->acAddText, &QAction::triggered, this, &This::addText);
    connect(ui->acDelete, &QAction::triggered, this, &This::doDelete);
    // Tools
    connect(ui->acAcceptChanges, &QAction::triggered, this, &This::acceptCurrObject);

    // Unused menu items
    ui->acMoveUp->setEnabled(false);
    ui->acMoveDown->setEnabled(false);

    reenable();
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
        reenable();
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
            acceptObject(*formerObj);
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

    std::u8string_view toText(QPlainTextEdit* x, std::string& cache)
        { return toText(x->toPlainText(), cache); }

    /// @todo [transl] There will be button “Translation is empty string”
    std::optional<std::u8string_view> toOptText(
            QPlainTextEdit* x, std::string& cache)
    {
        auto text = x->toPlainText();
        if (text.isEmpty())
            return std::nullopt;
        return toText(text, cache);
    }

    std::u8string_view toU8(QLineEdit* x, std::string& cache)
        { return str::toU8(x->text(), cache); }

}   // anon namespace


void FmMain::acceptObject(tr::UiObject& obj)
{
    std::string cache;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        obj.setId(toU8(ui->edId, cache), tr::Modify::YES);
        obj.setOriginal(toText(ui->memoOriginal, cache), tr::Modify::YES);
        obj.setAuthorsComment(toText(ui->memoComment, cache), tr::Modify::YES);
        break;
    case tr::PrjType::FULL_TRANSL:
        obj.setTranslation(toOptText(ui->memoTranslation, cache), tr::Modify::YES);
        obj.setTranslatorsComment(toText(ui->memoComment, cache), tr::Modify::YES);
        break;
    }
}


void FmMain::acceptCurrObject()
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    acceptObject(*obj);
}


void FmMain::reenable()
{
    bool isMainVisible = (ui->stackMain->currentWidget() == ui->pageMain);
    bool hasProject {project };
    bool isOriginal = (isMainVisible && hasProject
                       && project->info.type == tr::PrjType::ORIGINAL);
    //bool isTranslation = (hasProject && !isOriginal);

    // Menu: File
    ui->acSave->setEnabled(hasProject);
    ui->acSaveAs->setEnabled(hasProject);

    // Menu: Go
    ui->acGoBack->setEnabled(isMainVisible);
    ui->acGoNext->setEnabled(isMainVisible);

    // Menu: Original; always isOriginal
    ui->acAddHostedFile->setEnabled(isOriginal);
    ui->acAddHostedGroup->setEnabled(isOriginal);
    ui->acAddText->setEnabled(isOriginal);
    ui->acDelete->setEnabled(isOriginal);

    // Tools
    ui->acAcceptChanges->setEnabled(isMainVisible);
}


void FmMain::goBack()
{
    /// @todo [urgent] goPrev
    QMessageBox::information(this, "goBack", "goBack!!!!!");
}


void FmMain::goNext()
{
    /// @todo [urgent] goNext
    QMessageBox::information(this, "goNext", "goNext!!!!!");
}


void FmMain::startEditingOrig(const QModelIndex& index)
{
    ui->treeStrings->setCurrentIndex(index);
    ui->edId->setFocus();
    ui->edId->selectAll();
}


void FmMain::addHostedFile()
{
    auto file = treeModel.addHostedFile();
    startEditingOrig(file.index);
}


QModelIndex FmMain::treeIndex()
{
    auto sel = ui->treeStrings->selectionModel()->selection();
    if (sel.size() != 1)
        return {};
    auto& s0 = sel[0];
    if (s0.height() > 1)
        return {};
    return s0.topLeft();
}


std::optional<std::shared_ptr<tr::VirtualGroup>> FmMain::disambigGroup(std::u8string_view title)
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    auto pair = obj->additionParents();
    if (pair.is2()) {
        return fmDisambigPair.ensure(this).exec(title, pair);
    } else {
        return pair.first;
    }
}


void FmMain::addHostedGroup()
{
    if (auto dis = disambigGroup(u8"Add group")) {
        auto group = treeModel.addHostedGroup(*dis);
        if (group) {    // Have group
            startEditingOrig(group.index);
        } else {        // No group
            QMessageBox::warning(this,
                        "Add group",
                        "Select some file or group");
        }
    }
}


void FmMain::addText()
{
    if (auto dis = disambigGroup(u8"Add text")) {
        auto text = treeModel.addText(*dis);
        if (text) {    // Have text
            startEditingOrig(text.index);
        } else {        // No text
            QMessageBox::warning(this,
                        "Add text",
                        "Select some file or group");
        }
    }
}


void FmMain::doDelete()
{
    auto index = treeIndex();
    if (!index.isValid())
        return;
    auto obj = treeModel.toObjOr(index, nullptr);
    if (!obj)
        return;
    QString message;
    if (obj->objType() == tr::ObjType::TEXT) {
        message = "Delete text?";
    } else {
        /// @todo [future] what to do with plural rules?
        auto n = obj->nTexts();
        if (n == 1) {
            message = "Delete 1 text?";
        } else {
            message = QString("Delete %1 texts?").arg(n);
        }
    }
    auto answer = QMessageBox::question(this, "Delete", message);
    if (answer != QMessageBox::Yes)
        return;
    treeModel.extract(obj);
}
