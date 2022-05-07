// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// STL
#include <deque>

// Qt
#include <QItemSelectionModel>
#include <QMessageBox>

// Qt misc
#include "QModels.h"

// Libs
#include "u_Qstrings.h"
#include "i_OpenSave.h"

// Translation
#include "TrFinder.h"

// Project-local
#include "d_Config.h"
#include "d_Strings.h"

// UI forms
#include "FmNew.h"
#include "FmDisambigPair.h"
#include "FmDecoder.h"
#include "FmFileFormat.h"
#include "FmFind.h"


///// PrjTreeModel /////////////////////////////////////////////////////////////

namespace {
    enum {
        COL_ID = 0,
        COL_ORIG = 1,
        COL_TRANSL = 2,
        N_ORIG = COL_ORIG + 1,
        N_TRANSL = COL_TRANSL + 1,
    };

    constinit const tw::L10n treeL10n {
        .untranslated = S8(STR_UNTRANSLATED),
        .emptyString = S8(STR_EMPTY_STRING),
    };
}   // anon namespace


PrjTreeModel::PrjTreeModel()
{
    fly.setL10n(treeL10n);
}


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

    QString brushLineEnds(std::u8string_view x)
    {
        auto r = str::toQ(x);
        r.replace('\n', QChar{L'¶'});
        return r;
    }

    QString brushLineEnds(const tw::TranslObj& x)
    {
        auto r = str::toQ(x.str());
        if (x.mayContainEols()) {
            r.replace('\n', QChar{L'¶'});
        }
        return r;
    }

    constinit const QColor fgColors[] = {
        QColor(),               // NORMAL,
        { 0x77, 0x00, 0x00 },   // UNTRANSLATED_CAT — some dark red
        { 0xDC, 0x14, 0x3C },   // ATTENTION — HTML crimson
        { 0x00, 0x80, 0x00 },   // OK — dumb green
        { 0x69, 0x69, 0x69 },   // STATS — HTML dim gray
        { 0xD3, 0xD3, 0xD3 },   // LIGHT — HTML light gray
    };
    static_assert(std::size(fgColors) == tw::Fg_N);
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
                /// @todo [urgent] get rid of origColumn
                return brushLineEnds(obj->origColumn());
            case COL_TRANSL:
                return brushLineEnds(fly.getTransl(*obj));
            default:
                return {};
            }
        }
    case Qt::BackgroundRole: {
            auto obj = toObj(index);
            switch (index.column()) {
            case COL_ID:
                if (obj->cache.mod.has(tr::Mch::META_ID))
                    return BG_MODIFIED;
                return {};
            case COL_ORIG:
                if (obj->cache.mod.has(tr::Mch::ORIG))
                    return BG_MODIFIED;
                return {};
            case COL_TRANSL:
                if (obj->cache.mod.has(tr::Mch::TRANSL))
                    return BG_MODIFIED;
                return {};
            default:
                return {};
            }
        }
    case Qt::ForegroundRole: {
            auto obj = toObj(index);
            switch (index.column()) {
            case COL_TRANSL: {
                    auto& cl = fgColors[fly.getTransl(*obj).iFg()];
                    return cl.isValid() ? cl : QVariant();
                }
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

namespace {

    #define FILE_PREFIX u8"file"
    #define FILE_SUFFIX u8".txt"
    #define FILE_INITIAL   FILE_PREFIX "0" FILE_SUFFIX

    const tr::IdLib myIds {
        .filePrefix = FILE_PREFIX,
        .fileSuffix = FILE_SUFFIX,
        .groupPrefix = u8"g",
        .textPrefix = {}
    };

}   // anon namespace


Thing<tr::File> PrjTreeModel::addHostedFile()
{
    auto newId = project->makeId<tr::ObjType::FILE>(myIds);
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
    auto newId = parent->makeId<tr::ObjType::GROUP>(myIds);
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
    auto newId = parent->makeId<tr::ObjType::TEXT>(myIds);
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
    auto r = toObj(pntIndex)->extractChild(myIndex, tr::Modify::YES);
    endRemoveRows();
    if (!r)
        throw std::logic_error("[PrjTreeModel.doDelete] Identified as deletable, but did not delete");
    return r;
}


PrjTreeModel::CloneResult PrjTreeModel::doClone(const QModelIndex& index)
{
    if (!index.isValid())
        return { tr::CloneErr::BAD_OBJECT, {}, tr::ObjType::TEXT };
    auto obj = toObj(index);
    auto pnt = obj->parent();
    if (auto res = obj->startCloning(pnt)) {
        auto newIndex = pnt->nChildren();
        beginInsertRows(toIndex(pnt, 0), newIndex, newIndex);
        auto v = res.commit(&myIds, tr::Modify::YES);
        endInsertRows();
        return { tr::CloneErr::OK, toIndex(*v, 0), v->objType() };
    } else {
        return { res.err, {}, tr::ObjType::TEXT };
    }
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


auto PrjTreeModel::moveUp(const QModelIndex& index) -> MoveResult
{
    auto child = toObj(index);
    auto parent = child->parent();
    if (!parent->canMoveUp(child))
        return {};
    auto indParent = toIndex(parent, 0);
    if (!beginMoveRows(indParent, index.row(), index.row(), indParent, index.row() - 1))
        return {};
    parent->moveUp(child);
    endMoveRows();
    child->checkCanary();
    auto mainInd = toIndex(child, 0);
    if (child->cache.index == 0) {
        auto parentInd = toIndex(parent, 0);
        if (parentInd.isValid()) {
            return { mainInd, parentInd };
        } else {
            return { mainInd, mainInd };
        }
    } else {
        return { mainInd, toIndex(parent->child(child->cache.index - 1).get(), 0) };
    }
}


QModelIndex PrjTreeModel::clearGroup(tr::UiObject* obj)
{
    if (!obj)
        return {};
    obj->checkCanary();
    auto index = toIndex(obj, 0);
    auto nCh = obj->nChildren();
    if (nCh != 0 && index.isValid()) {
        beginRemoveRows(index, 0, nCh - 1);
        obj->clearChildren();
        obj->doModify(tr::Mch::META);
        endRemoveRows();
    }
    return index;
}


auto PrjTreeModel::moveDown(const QModelIndex& index) -> MoveResult
{
    auto child = toObj(index);
    auto parent = child->parent();
    if (!parent->canMoveDown(child))
        return {};
    auto indParent = toIndex(parent, 0);
    // See doc — both index and index+1 are IMMOBILE
    if (!beginMoveRows(indParent, index.row(), index.row(), indParent, index.row() + 2))
        return {};
    parent->moveDown(child);
    endMoveRows();
    child->checkCanary();
    auto mainInd = toIndex(child, 0);
    if (static_cast<size_t>(child->cache.index) + 1 >= parent->nChildren()) {
        return { mainInd, mainInd };
    } else {
        return { mainInd, toIndex(parent->child(child->cache.index + 1), 0) };
    }
}



///// FmMain ///////////////////////////////////////////////////////////////////

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);
    config::history.setListener(this);


    ui->wiFind->hide();
    dismissChanges();

    // Splitter
    auto h = height();
    ui->splitMain->setSizes({ h, 1 });
    ui->splitMain->setStretchFactor(0, 2);
    ui->splitMain->setStretchFactor(1, 1);

    // Model
    ui->treeStrings->setModel(&treeModel);
    ui->treeStrings->setItemDelegate(&treeModel);

    // Signals/slots: tree etc.
    connect(ui->treeStrings->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &This::treeCurrentChanged);

    // Signals/slots: editing
    connect(ui->edId, &QLineEdit::textEdited, this, &This::tempModify);
    connect(ui->memoOriginal, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->chkIdless, &QCheckBox::clicked, this, &This::tempModify);
    connect(ui->memoTranslation, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->memoComment, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->btFileFormat, &QPushButton::clicked, this, &This::editFileFormat);

    // Signals/slots: search
    connect(ui->wiFind, &WiFind::closed, this, &This::searchClosed);
    connect(ui->wiFind, &WiFind::indexChanged, this, &This::searchChanged);

    // Signals/slots: menu
    // Starting screen
    connect(ui->btStartEdit, &QPushButton::clicked, this, &This::goEdit);
    connect(ui->btStartNew, &QPushButton::clicked, this, &This::doNew);
    connect(ui->btStartOpen, &QPushButton::clicked, this, &This::doOpen);
    connect(ui->browStart, &QTextBrowser::anchorClicked, this, &This::startLinkClicked);
    // File
    connect(ui->acNew, &QAction::triggered, this, &This::doNew);
    connect(ui->acOpen, &QAction::triggered, this, &This::doOpen);
    connect(ui->acSave, &QAction::triggered, this, &This::doSave);
    connect(ui->acSaveAs, &QAction::triggered, this, &This::doSaveAs);
    connect(ui->acStartingScreen, &QAction::triggered, this, &This::goToggleStart);
    // Original
    connect(ui->acAddHostedFile, &QAction::triggered, this, &This::addHostedFile);
    connect(ui->acAddHostedGroup, &QAction::triggered, this, &This::addHostedGroup);
    connect(ui->acAddText, &QAction::triggered, this, &This::addText);
    connect(ui->acDelete, &QAction::triggered, this, &This::doDelete);
    connect(ui->acClone, &QAction::triggered, this, &This::doClone);
    connect(ui->acMoveUp, &QAction::triggered, this, &This::doMoveUp);
    connect(ui->acMoveDown, &QAction::triggered, this, &This::doMoveDown);
    connect(ui->acClearGroup, &QAction::triggered, this, &This::clearGroup);
    connect(ui->acLoadTexts, &QAction::triggered, this, &This::doLoadText);
    // Edit
    connect(ui->acAcceptChanges, &QAction::triggered, this, &This::acceptCurrObject);
    connect(ui->acRevertChanges, &QAction::triggered, this, &This::revertCurrObject);
    // Go
    connect(ui->acGoBack, &QAction::triggered, this, &This::goBack);
    connect(ui->acGoNext, &QAction::triggered, this, &This::goNext);
    connect(ui->acGoUp, &QAction::triggered, this, &This::goUp);
    connect(ui->acGoFind, &QAction::triggered, this, &This::goFind);
    connect(ui->acGoFindNext, &QAction::triggered, ui->wiFind, &WiFind::goNext);
    connect(ui->acGoFindPrev, &QAction::triggered, ui->wiFind, &WiFind::goBack);
    connect(ui->acGoCloseSearch, &QAction::triggered, ui->wiFind, &WiFind::close);
    // Tools
    connect(ui->acDecoder, &QAction::triggered, this, &This::runDecoder);

    // Unused parts
    ui->grpCompatId->hide();

    setEditorsEnabled(false);   // while no project, let it be false
    updateCaption();

    goStart();
}

FmMain::~FmMain()
{
    delete ui;
}


void FmMain::selectSmth()
{
    ui->treeStrings->setFocus(Qt::FocusReason::OtherFocusReason);
    auto index = qmod::selectFirstAmbiguous(ui->treeStrings);
    ui->treeStrings->expand(index);
}


void FmMain::plantNewProject(std::shared_ptr<tr::Project>&& x)
{
    project = std::move(x);
    project->setStaticModifyListener(this);
    treeModel.setProject(project);
    ui->stackMain->setCurrentWidget(ui->pageMain);
    adaptLayout();
    updateCaption();
    selectSmth();
    reenable();
}


void FmMain::doNew()
{
    if (!checkSave("New"))
        return;
    if (auto result = fmNew.ensure(this).exec(FILE_INITIAL)) {
        plantNewProject(std::move(result));
    }
}


void FmMain::adaptLayout()
{
    ui->wiId->setVisible(project->info.canAddFiles());
    ui->grpTranslation->setVisible(project->info.isTranslation());
}


void FmMain::setEditorsEnabled(bool x)
{
    ui->wiEditors->setEnabled(x);
    if (!x)
        ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
}


void FmMain::treeCurrentChanged(
        const QModelIndex& current, const QModelIndex& former)
{
    auto formerObj = treeModel.toObj(former);
    auto currentObj = treeModel.toObj(current);

    if (current.isValid()) {
        if (auto currentPrj = currentObj->project()) {
            if (formerObj->project() == currentPrj)
                acceptObject(*formerObj);
            setEditorsEnabled(true);
            loadObject(*currentObj);
            return;
        }
    }
    // Otherwise
    setEditorsEnabled(false);
}


void FmMain::setMemo(QWidget* wi, QPlainTextEdit* memo, std::u8string_view y)
{
    wi->setEnabled(true);
    isChangingProgrammatically = true;
    memo->setPlainText(str::toQ(y));
    isChangingProgrammatically = false;
}

void FmMain::banMemo(QWidget* wi, QPlainTextEdit* memo)
{
    wi->setEnabled(false);
    isChangingProgrammatically = true;
    memo->clear();
    isChangingProgrammatically = false;
}


void FmMain::loadContext(tr::UiObject* lastSon)
{
    // Build HTML backwards
    QString html = "</dl>";
    for (auto p = lastSon;
            p && p->objType() != tr::ObjType::PROJECT;
            p = p->parent().get()) {
        auto itsText = u8"<dt>•&nbsp;" + str::toQ(p->idColumn()).toHtmlEscaped() + "</dt>";
        if (auto cmt = p->comments()) {
            if (!cmt->authors.empty()) {
                itsText += "<dd>";
                auto text = str::toQ(cmt->authors).toHtmlEscaped();
                text.replace("\n", "<br>");
                itsText += text;
                itsText += "</dd>";
            }
        }
        html = itsText + html;
    }
    html = "<dl>" + html;
    ui->browContext->setHtml(html);
}


void FmMain::loadObject(tr::UiObject& obj)
{
    ui->edId->setText(str::toQ(obj.idColumn()));
    // File/Original/translation
    if (auto fi = obj.ownFileInfo()) {
        ui->stackOriginal->setCurrentWidget(ui->pageFile);
        ui->chkIdless->setChecked(fi->isIdless);
        bool canAddFiles = project->info.canAddFiles();
        ui->pageFile->setEnabled(canAddFiles);
        ui->wiId->show();
        ui->wiId->setEnabled(canAddFiles);
    } else if (auto tr = obj.translatable()) {      // mutually exclusive with fileInfo
        if (project->info.canEditOriginal()) {
            ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
            setMemo(ui->grpOriginal, ui->memoOriginal, tr->original);
        } else {
            ui->stackOriginal->setCurrentWidget(ui->pageUneditableOriginal);
            ui->richedOriginal->setPlainText(str::toQ(tr->original));
        }
        setMemo(ui->grpTranslation, ui->memoTranslation, tr->translationSv());
    } else {
        ui->wiId->setVisible(project->info.canEditOriginal());
        ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
        ui->grpOriginal->setEnabled(false);
        ui->grpTranslation->setEnabled(false);
        banMemo(ui->grpOriginal, ui->memoOriginal);
        banMemo(ui->grpTranslation, ui->memoTranslation);
    }
    // Comment
    if (auto comm = obj.comments()) {
        if (project->info.canEditOriginal()) {
            // Bilingual (currently unimplemented):
            // can edit original → author’s comment; cannot → translator’s
            setMemo(ui->grpComment, ui->memoComment, comm->authors);
        } else {
            setMemo(ui->grpComment, ui->memoComment, comm->translators);
        }
    } else {
        banMemo(ui->grpComment, ui->memoComment);
    }
    // Context
    if (project->info.canEditOriginal()) {
        // If we edit original → load parent
        loadContext(obj.parent().get());
    } else {
        // Otherwise load self!
        loadContext(&obj);
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

    std::u8string_view toTextSv(QString x, std::string& cache)
    {
        normalizeEol(x);
        cache = x.toStdString();
        return { reinterpret_cast<const char8_t*>(cache.data()), cache.length() };
    }

    std::u8string_view toTextSv(QPlainTextEdit* x, std::string& cache)
        { return toTextSv(x->toPlainText(), cache); }

    /// @todo [transl] There will be button “Translation is empty string”
    std::optional<std::u8string_view> toOptTextSv(
            QPlainTextEdit* x, std::string& cache)
    {
        auto text = x->toPlainText();
        if (text.isEmpty())
            return std::nullopt;
        return toTextSv(text, cache);
    }

    std::u8string_view toU8sv(QLineEdit* x, std::string& cache)
        { return str::toU8sv(x->text(), cache); }

}   // anon namespace


void FmMain::acceptObject(tr::UiObject& obj)
{
    auto idx0 = treeModel.toIndex(obj, 0);
    auto idx9 = treeModel.toIndex(obj, treeModel.columnCount() - 1);
    treeModel.dataChanged(idx0, idx9);
    std::string cache;
    if (project->info.canEditOriginal()) {
        obj.setId(toU8sv(ui->edId, cache), tr::Modify::YES);
        obj.setIdless(ui->chkIdless->isChecked(), tr::Modify::YES);
        obj.setOriginal(toTextSv(ui->memoOriginal, cache), tr::Modify::YES);
        // Bilingual (currently unimplemented):
        // can edit original → author’s comment; cannot → translator’s
        obj.setAuthorsComment(toTextSv(ui->memoComment, cache), tr::Modify::YES);
    } else {
        obj.setTranslatorsComment(toTextSv(ui->memoComment, cache), tr::Modify::YES);
    }
    if (project->info.isTranslation()) {
        obj.setTranslation(toOptTextSv(ui->memoTranslation, cache), tr::Modify::YES);
    }
    if (project)
        project->tempRevert();
}


tr::UiObject* FmMain::acceptCurrObject()
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    acceptObject(*obj);
    return obj;
}


void FmMain::tempModify()
{
    if (project && !isChangingProgrammatically)
        project->tempModify();
}


void FmMain::revertCurrObject()
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    loadObject(*obj);
    if (project)
        project->tempRevert();
    selectSmth();
}


void FmMain::reenable()
{
    bool isStartVisible = (ui->stackMain->currentWidget() == ui->pageStart);
    bool isMainVisible = (ui->stackMain->currentWidget() == ui->pageMain);
    bool hasProject { project };
    bool isOriginal = (isMainVisible && hasProject
                       && project->info.canEditOriginal());
    bool canAddFiles = (isMainVisible && hasProject
                       && project->info.canAddFiles());
    bool canSearch = ui->wiFind->isVisible() && isMainVisible;

    // Starting screen
    ui->btStartEdit->setVisible(hasProject);

    // Menu: File
        // New, Open are always available
    ui->acSave->setEnabled(hasProject);
    ui->acSaveAs->setEnabled(hasProject);
    ui->acStartingScreen->setEnabled(hasProject);
        ui->acStartingScreen->setChecked(hasProject && isStartVisible);

    // Menu: Original; always isOriginal
    ui->acAddHostedFile->setEnabled(canAddFiles);
    ui->acAddHostedGroup->setEnabled(isOriginal);
    ui->acAddText->setEnabled(isOriginal);
    ui->acDelete->setEnabled(canAddFiles);
    ui->acClone->setEnabled(isOriginal);
    ui->acClearGroup->setEnabled(isOriginal);
    ui->acMoveUp->setEnabled(isOriginal);
    ui->acMoveDown->setEnabled(isOriginal);
    ui->acLoadTexts->setEnabled(isOriginal);

    // Menu: Edit
    ui->acAcceptChanges->setEnabled(isMainVisible);
    ui->acRevertChanges->setEnabled(isMainVisible);

    // Menu: Go
    ui->acGoBack->setEnabled(isMainVisible);
    ui->acGoNext->setEnabled(isMainVisible);
    ui->acGoUp->setEnabled(isMainVisible);
    ui->acGoFind->setEnabled(isMainVisible);
    ui->acGoFindNext->setEnabled(canSearch);
    ui->acGoFindPrev->setEnabled(canSearch);
    ui->acGoCloseSearch->setEnabled(canSearch);
}


void FmMain::goBack()
{
    qmod::LeafPos cp(ui->treeStrings->currentIndex());
    if (cp.goPrev())
        ui->treeStrings->setCurrentIndex(cp);
}


void FmMain::goNext()
{
    qmod::LeafPos cp(ui->treeStrings->currentIndex());
    if (cp.goNext())
        ui->treeStrings->setCurrentIndex(cp);
}


void FmMain::goUp()
{
    auto index = ui->treeStrings->currentIndex();
    if (!index.isValid())
        return;
    auto parent = treeModel.parent(index);
    if (!parent.isValid())
        return;
    ui->treeStrings->setCurrentIndex(parent);
}


void FmMain::startEditingOrig(const QModelIndex& index, EditMode editMode)
{
    ui->treeStrings->setCurrentIndex(index);
    auto obj = treeModel.toObj(index);
    auto file = obj->file();
    switch (editMode) {
    case EditMode::TEXT:
        if (file && file->info.isIdless) {
            ui->memoOriginal->setFocus();
            break;
        }
        [[fallthrough]];
    case EditMode::GROUP:
        ui->edId->setFocus();
        ui->edId->selectAll();
        break;
    }
}


void FmMain::addHostedFile()
{
    auto file = treeModel.addHostedFile();
    startEditingOrig(file.index, EditMode::GROUP);
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
    auto obj = acceptCurrObject();
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
            startEditingOrig(group.index, EditMode::GROUP);
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
            startEditingOrig(text.index, EditMode::TEXT);
        } else {        // No text
            QMessageBox::warning(this,
                        "Add text",
                        "Select some file or group");
        }
    }
}


void FmMain::doDelete()
{
    /// @todo [freestyle, #12] Freestyle translation can add/remove files only, not groups
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
        auto& stats = obj->stats(tr::StatsMode::CACHED, tr::CascadeDropCache::YES);
        switch (stats.nTexts) {
        case 0:
            if (stats.nGroups == 0)
                message = "Delete empty group?";
                else message = "Delete group w/o texts?";
            break;
        /// @todo [future] what to do with plural rules?
        case 1: message = "Delete 1 text?"; break;
        default: message = QString("Delete %1 texts?").arg(stats.nTexts);
        }

    }
    auto answer = QMessageBox::question(this, "Delete", message);
    if (answer != QMessageBox::Yes)
        return;
    /// @todo [find, #20] limit search??
    ui->wiFind->close();
    treeModel.extract(obj);
}


void FmMain::modStateChanged(ModState oldState, ModState newState)
{
    if (newState == ModState::UNMOD && oldState == ModState::MOD)
        treeModel.dataChanged({}, {});
    updateCaption();
}


void FmMain::updateCaption()
{
    QString s;
    if (project) {
        switch (project->modState()) {
        case ModState::UNMOD: break;
        case ModState::TEMP: s += u8"? "; break;
        case ModState::MOD: s += u8"✱ "; break;
        }
        str::append(s, project->shownFname(S8(STR_UNTITLED)));
        s += u8" · ";
    }
    s += "UTranslator";
    setWindowTitle(s);
}


bool FmMain::doSaveAs()
{
    if (!project)
        return false;
    filedlg::Filters filters;
    const wchar_t* extension = nullptr;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        filters.emplace_back(L"Originals", L"*.uorig");
        extension = L".uorig";
        break;
    case tr::PrjType::FULL_TRANSL:
        filters.emplace_back(L"Translations", L"*.utran");
        extension = L".utran";
        break;
    }
    filters.emplace_back(L"All files", L"*");

    acceptCurrObject();
    auto fname = filedlg::save(
                this, nullptr, filters, extension, {},
                filedlg::AddToRecent::YES);
    if (fname.empty())
        return false;
    try {
        project->save(fname);
        config::history.pushFile(project->fname);
        doBuild();
        return true;
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Save", QString::fromStdString(e.what()));
        return false;
    }
}


void FmMain::openFile(std::filesystem::path fname)      // by-value + move
{
    dismissChanges();
    auto prj = tr::Project::make();
    try {
        prj->load(fname);
        plantNewProject(std::move(prj));
        config::history.pushFile(std::move(fname));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Open", QString::fromStdString(e.what()));
    }
}


void FmMain::doOpen()
{
    if (!checkSave("Open"))
        return;
    filedlg::Filters filters
      { { L"UTranslator files", L"*.uorig *.utran" }, filedlg::ALL_FILES };
    auto fname = filedlg::open(
                this, nullptr, filters, {},
                filedlg::AddToRecent::YES);
    if (!fname.empty()) {
        openFile(fname);
    }
}


bool FmMain::doSave()
{
    if (!project)
        return false;
    if (project->fname.empty()) {
        return doSaveAs();
    } else {
        acceptCurrObject();
        dismissChanges();
        try {
            project->save();
            config::history.pushFile(project->fname);
            doBuild();
            return true;
        } catch (std::exception& e) {
            QMessageBox::critical(this, "Save problem", QString::fromStdString(e.what()));
            return false;
        }
    }
}


void FmMain::runDecoder()
{
    std::unique_ptr<QstrObject> obj;
    if (ui->memoOriginal->hasFocus()) {
        obj = std::make_unique<MemoObject>(ui->memoOriginal);
    } else if (ui->memoTranslation->hasFocus()) {
        obj = std::make_unique<MemoObject>(ui->memoTranslation);
    } else if (ui->memoComment->hasFocus()) {
        obj = std::make_unique<MemoObject>(ui->memoComment);
    }
    fmDecoder.ensure(this).exec(obj.get());
}


void FmMain::doClone()
{
    auto res = treeModel.doClone(treeIndex());
    if (res.index.isValid()) {
        ui->treeStrings->expand(res.index);
        startEditingOrig(res.index,
                (res.objType == tr::ObjType::TEXT) ? EditMode::TEXT : EditMode::GROUP);
    }
    switch (res.err) {
    case tr::CloneErr::OK:
        break;
    case tr::CloneErr::BAD_PARENT:
        QMessageBox::critical(this, "Clone", "Bad parent (strange error).");
        break;
    case tr::CloneErr::BAD_OBJECT:
        QMessageBox::warning(this, "Clone", "Select some object.");
        break;
    case tr::CloneErr::UNCLONEABLE:
        QMessageBox::warning(this, "Clone", "Cannot clone files.");
        break;
    }
}


void FmMain::historyChanged()
{
    QString html;
    int i = 0;
    html += "<style>"
            ".a_big { "
                "color: #4169E1; "
                "text-decoration: none; "
            " }"
            "</style>";
    html += "<table border='0' cellspacing='6'>";
    for (auto& v : config::history) {
        html += "<tr>"
            // 1st cell
                "<td style='font-size:14pt'>";
        html += QString::number(i + 1);
        if (i < 10)
            html += "&nbsp;";
        QUrl url;
            url.setScheme("hist");
            url.setPath(QString::number(i));
        html += "&nbsp;</td>"
            // 2nd cell — top
                "<td><span style='font-size:14pt'><a class='a_big' href='";
            html += url.toString();
            html += "'>";
        html += QString::fromStdWString(v->shortName()).toHtmlEscaped();
        html += "</span><br><span style='color:#606060'>";
        html += QString::fromStdWString(v->auxName()).toHtmlEscaped();
        html += "</span></td>";
        ++i;
    }
    ui->browStart->setHtml(html);
}


void FmMain::goToggleStart()
{
    if (ui->stackMain->currentWidget() == ui->pageStart) {
        goEdit();
    } else {
        goStart();
    }
}


void FmMain::goEdit()
{
    ui->stackMain->setCurrentWidget(ui->pageMain);
    reenable();
}


void FmMain::goStart()
{
    ui->stackMain->setCurrentWidget(ui->pageStart);
    ui->pageStart->setFocus();
    reenable();
}


void FmMain::startLinkClicked(QUrl url)
{
    if (url.scheme() == "hist") {
        QString s = url.path();
        bool isOk = false;
        auto i = s.toInt(&isOk);
        if (isOk) {
            if (auto place = config::history[i]) {
                if (checkSave("Open")) {
                    if (auto fplace = std::dynamic_pointer_cast<hist::FilePlace>(place)) {
                        openFile(fplace->path());
                    }
                }
            }
        }
    }
}


void FmMain::editFileFormat()
{    
    auto index = ui->treeStrings->currentIndex();
    tr::UiObject* obj = treeModel.toObj(index);
    if (auto format = obj->ownFileFormat()) {
        auto nExOld = project->nOrigExportableFiles();
        bool isOk = fmFileFormat.ensure(this).exec(
                    *format,
                    static_cast<tf::LoadTextsSettings*>(nullptr),
                    *obj->allowedFormats());
        if (isOk) {
            obj->doModify(tr::Mch::META);
            if (nExOld == 0 && project->nOrigExportableFiles() != 0) {
                QMessageBox::information(this, "Information",
                    "You have created your first exportable file.\n"
                    "Press “Save”, and UTranslator will also build your localization.");
            }
        }
    }
}


namespace {

    class Finder : public tr::TraverseListener
    {
    public:
        Finder(const FindOptions& aOpts);
        void onText(const std::shared_ptr<tr::Text>&) override;
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        std::unique_ptr<ts::Result> give() { return std::move(r); }
        bool isEmpty() const { return r->isEmpty(); }
    private:
        FindOptions opts;
        std::unique_ptr<ts::Result> r;
        Qt::CaseSensitivity caseSen;
        bool matchEntity(const tr::Entity& x);
        bool matchText(const tr::Text& x);
    };

    Finder::Finder(const FindOptions& aOpts)
        : opts{aOpts}, r{std::make_unique<ts::Result>()}
    {
        caseSen = opts.options.matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
    }

    bool Finder::matchEntity(const tr::Entity& x)
    {
        if (opts.channels.id
                && str::toQ(x.id).indexOf(opts.text, caseSen) >= 0)
            return true;
        if (opts.channels.authorsComment
                && str::toQ(x.comm.authors).indexOf(opts.text, caseSen) >= 0)
            return true;
        if (opts.channels.translatorsComment
                && str::toQ(x.comm.translators).indexOf(opts.text, caseSen) >= 0)
            return true;
        return false;
    }

    bool Finder::matchText(const tr::Text& x)
    {
        if (opts.channels.original
                && str::toQ(x.tr.original).indexOf(opts.text, caseSen) >= 0)
            return true;
        if (opts.channels.translation
                && str::toQ(x.tr.translationSv()).indexOf(opts.text, caseSen) >= 0)
            return true;
        return false;
    }

    void Finder::onText(const std::shared_ptr<tr::Text>& x)
    {
        if (matchEntity(*x) || matchText(*x))
            r->add(x);
    }

    void Finder::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x)
    {
        if (matchEntity(*x))
            r->add(x);
    }

}   // anon namespace


void FmMain::goFind()
{
    if (auto opts = fmFind.ensure(this).exec(project->info.type)) {
        Finder finder(opts);
        project->traverse(finder, tr::WalkOrder::EXACT, tr::EnterMe::NO);
        QString caption = QString{"Find “%1”"}.arg(opts.text);
        plantSearchResult(caption, finder.give());
    }
}


void FmMain::plantSearchResult(
        const QString& caption, std::unique_ptr<ts::Result> x)
{
    if (!x || x->isEmpty()) {
        ui->wiFind->close();    // automatically reenables
        QMessageBox::information(this, "Find", "Not found.");
    } else {
        searchResult = std::move(x);
            // will not reenable for now
        ui->wiFind->startSearch(caption, searchResult->size());
        reenable();
    }
}


void FmMain::doBuild()
{
    if (!project)
        return;
    project->doBuild({});
}


void FmMain::doMoveUp()
{
    QModelIndex index = ui->treeStrings->currentIndex();
    if (auto r = treeModel.moveUp(index)) {
        ui->treeStrings->setCurrentIndex(r.that);
        ui->treeStrings->scrollTo(r.next);
        ui->treeStrings->scrollTo(r.that);
    }
}


void FmMain::doMoveDown()
{
    QModelIndex index = ui->treeStrings->currentIndex();
    if (auto r = treeModel.moveDown(index)) {
        ui->treeStrings->setCurrentIndex(r.that);
        ui->treeStrings->scrollTo(r.next);
        ui->treeStrings->scrollTo(r.that);
    }
}


void FmMain::clearGroup()
{
    QModelIndex index = ui->treeStrings->currentIndex();
    auto obj = treeModel.toObj(index);
    auto selectedGroup = obj->nearestGroup();
    if (!selectedGroup) {
        QMessageBox::warning(this, "Clear group", "Please select something!");
        return;
    }

    QString text = QString(u8"Clear group “%1”?").arg(str::toQ(selectedGroup->idColumn()));
    if (QMessageBox::question(this, "Clear group", text) == QMessageBox::Yes) {
        auto newIndex = treeModel.clearGroup(selectedGroup.get());
        if (newIndex.isValid())
            ui->treeStrings->setCurrentIndex(newIndex);
    }
}


void FmMain::doLoadText()
{
    QModelIndex index = ui->treeStrings->currentIndex();
    auto obj = treeModel.toObj(index);
    auto selectedGroup = obj->nearestGroup();
    auto file = obj->file();
    if (!selectedGroup || !file) {
        QMessageBox::warning(this, "Load texts", "Please select something!");
        return;
    }

    acceptCurrObject();

    // Get current file
    CloningUptr<tf::FileFormat> fileFormat;
    if (file.get() != loadSetsCache.fileKey) {
        if (file->info.format)
            fileFormat = file->info.format->clone();
    }
    if (!fileFormat && loadSetsCache.format) {
        fileFormat = loadSetsCache.format->clone();
    }

    if (fmFileFormat.ensure(this).exec(
                fileFormat, &loadSetsCache.text,
                tf::ProtoFilter::ALL_IMPORTING)) {
        // Save as soon as we chose smth, even if we press Cancel afterwards
        loadSetsCache.format = std::move(fileFormat);
        loadSetsCache.fileKey = file.get();
        auto filter = loadSetsCache.format->fileFilter();
        filedlg::Filters filters { filter, filedlg::ALL_FILES };
        std::filesystem::path fileName = filedlg::open(
                this, {}, filters, filter.extension(),
                filedlg::AddToRecent::NO);

        if (!fileName.empty()) {
            std::shared_ptr<tr::VirtualGroup> destGroup;
            switch (loadSetsCache.text.loadTo) {
            case tf::LoadTo::ROOT:
                destGroup = file;
                break;
            case tf::LoadTo::SELECTED:
                destGroup = selectedGroup;
                break;
            }

            try {
                auto thing = treeModel.lock();
                destGroup->loadText(*loadSetsCache.format, fileName,
                                    loadSetsCache.text.existing);
            } catch (std::exception& e) {
                QMessageBox::critical(this, "Load texts", QString::fromStdString(e.what()));
            }
        }
    }
}


bool FmMain::checkSave(std::string_view caption)
{
    if (!project || !project->isModified())
        return true;

    QString text = "Save untitled file?";
    auto shname = project->shownFname({});
    if (!shname.empty()) {
        text = QString{u8"Save “%1”?"}.arg(str::toQ(shname));
    }
    static constexpr auto SAVE    = QMessageBox::Save;
    static constexpr auto DISCARD = QMessageBox::Discard;
    static constexpr auto CANCEL  = QMessageBox::Cancel;
    auto result = QMessageBox::question(
                this, str::toQ(caption), text,
                SAVE | DISCARD | CANCEL);
    switch (result) {
    case SAVE:
        return doSave();
    case DISCARD:
        return true;
    default:
        return false;
    }
}


void FmMain::closeEvent(QCloseEvent *event)
{
    event->setAccepted(checkSave("Exit"));
}


void FmMain::searchClosed()
{
    searchResult.reset();
    reenable();
}


void FmMain::searchChanged(size_t index)
{
    if (searchResult) {
        if (auto obj = searchResult->at(index)) {
            if (auto index = treeModel.toIndex(obj, 0);
                    index.isValid()) {
                ui->treeStrings->setCurrentIndex(index);
            }
        }
    }
}


void FmMain::dismissChanges()
{
    ui->wiUpdate->hide();
}
