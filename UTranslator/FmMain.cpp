// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Qt
#include <QItemSelectionModel>
#include <QMessageBox>

// Qt misc
#include "QModels.h"

// Libs
#include "u_Qstrings.h"
#include "i_OpenSave.h"

// Project-local
#include "d_Config.h"

// UI forms
#include "FmNew.h"
#include "FmDisambigPair.h"
#include "FmDecoder.h"


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
                if (obj->cache.mod.has(tr::Mch::ID))
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

    const tr::IdLib myIds {
        .filePrefix = u8"file",
        .fileSuffix = u8"txt",
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
    auto r = toObj(pntIndex)->extractChild(myIndex);
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


///// FmMain ///////////////////////////////////////////////////////////////////

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);
    config::history.setListener(this);

    // Splitter
    auto h = height();
    ui->splitMain->setSizes({ h, 1 });
    ui->splitMain->setStretchFactor(0, 2);
    ui->splitMain->setStretchFactor(1, 1);

    // Helps
    ui->wiOriginalHelp->hide();

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
    // Edit
    connect(ui->acAcceptChanges, &QAction::triggered, this, &This::acceptCurrObject);
    connect(ui->acRevertChanges, &QAction::triggered, this, &This::revertCurrObject);
    // Go
    connect(ui->acGoBack, &QAction::triggered, this, &This::goBack);
    connect(ui->acGoNext, &QAction::triggered, this, &This::goNext);
    connect(ui->acGoUp, &QAction::triggered, this, &This::goUp);
    // Tools
    connect(ui->acDecoder, &QAction::triggered, this, &This::runDecoder);

    // Unused menu items
    ui->acMoveUp->setEnabled(false);
    ui->acMoveDown->setEnabled(false);

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
    if (auto result = fmNew.ensure(this).exec(0)) {
        auto x = tr::Project::make(std::move(*result));
        plantNewProject(std::move(x));
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


void FmMain::loadObject(tr::UiObject& obj)
{
    ui->edId->setText(str::toQ(obj.idColumn()));
    // File/Original/translation
    if (auto fi = obj.ownFileInfo()) {
        ui->stackOriginal->setCurrentWidget(ui->pageFile);
        ui->chkIdless->setChecked(fi->isIdless);
    } else if (auto tr = obj.translatable()) {      // mutually exclusive with fileInfo
        ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
        setMemo(ui->grpOriginal, ui->memoOriginal, tr->original);
        setMemo(ui->grpTranslation, ui->memoTranslation, tr->translationSv());
    } else {
        ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
        ui->grpOriginal->setEnabled(false);
        ui->grpTranslation->setEnabled(false);
        banMemo(ui->grpOriginal, ui->memoOriginal);
        banMemo(ui->grpTranslation, ui->memoTranslation);
    }
    // Comment
    if (auto comm = obj.comments()) {
        switch (project->info.type) {
        case tr::PrjType::ORIGINAL:
            setMemo(ui->grpComment, ui->memoComment, comm->authors);
            break;
        case tr::PrjType::FULL_TRANSL:
            setMemo(ui->grpComment, ui->memoComment, comm->translators);
            break;
        }
    } else {
        banMemo(ui->grpComment, ui->memoComment);
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
    auto idx0 = treeModel.toIndex(obj, 0);
    auto idx9 = treeModel.toIndex(obj, treeModel.columnCount() - 1);
    treeModel.dataChanged(idx0, idx9);
    std::string cache;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        obj.setId(toU8(ui->edId, cache), tr::Modify::YES);
        obj.setIdless(ui->chkIdless->isChecked(), tr::Modify::YES);
        obj.setOriginal(toText(ui->memoOriginal, cache), tr::Modify::YES);
        obj.setAuthorsComment(toText(ui->memoComment, cache), tr::Modify::YES);
        break;
    case tr::PrjType::FULL_TRANSL:
        obj.setTranslation(toOptText(ui->memoTranslation, cache), tr::Modify::YES);
        obj.setTranslatorsComment(toText(ui->memoComment, cache), tr::Modify::YES);
        break;
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
                       && project->info.type == tr::PrjType::ORIGINAL);

    // Starting screen
    ui->btStartEdit->setVisible(hasProject);

    // Menu: File
        // New, Open are always available
    ui->acSave->setEnabled(hasProject);
    ui->acSaveAs->setEnabled(hasProject);
    ui->acStartingScreen->setEnabled(hasProject);
        ui->acStartingScreen->setChecked(hasProject && isStartVisible);

    // Menu: Original; always isOriginal
    ui->acAddHostedFile->setEnabled(isOriginal);
    ui->acAddHostedGroup->setEnabled(isOriginal);
    ui->acAddText->setEnabled(isOriginal);
    ui->acDelete->setEnabled(isOriginal);
    ui->acClone->setEnabled(isOriginal);

    // Edit
    ui->acAcceptChanges->setEnabled(isMainVisible);
    ui->acRevertChanges->setEnabled(isMainVisible);

    // Menu: Go
    ui->acGoBack->setEnabled(isMainVisible);
    ui->acGoNext->setEnabled(isMainVisible);
    ui->acGoUp->setEnabled(isMainVisible);
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
    ui->treeStrings->selectionModel()->setCurrentIndex(
                parent, QItemSelectionModel::SelectCurrent);
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
        auto stats = obj->stats(false);
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
        auto fname = project->fname.filename().u8string();
        if (fname.empty()) {
            s += "(Untitled)";
        } else {
            str::append(s, fname);
        }
        s += u8" · ";
    }
    s += "UTranslator";
    setWindowTitle(s);
}


void FmMain::doSaveAs()
{
    if (!project)
        return;
    filedlg::Filters filters;
    const wchar_t* extension = nullptr;
    switch (project->info.type) {
    case tr::PrjType::ORIGINAL:
        filters.emplace_back(L"Originals", L"*.uorig");
        extension = L".uorig";
        break;
    case tr::PrjType::FULL_TRANSL:
        filters.emplace_back(L"Full translations", L"*.ufull");
        extension = L".ufull";
        break;
    }
    filters.emplace_back(L"All files", L"*");

    acceptCurrObject();
    auto fname = filedlg::save(
                this, nullptr, filters, extension, {},
                filedlg::AddToRecent::YES);
    if (fname.empty())
        return;
    try {
        project->save(fname);
        config::history.pushFile(project->fname);
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Save", QString::fromStdString(e.what()));
    }
}


void FmMain::openFile(std::filesystem::path fname)
{
    auto prj = tr::Project::make();
    try {
        prj->load(fname);
        plantNewProject(std::move(prj));
        config::history.pushFile(std::move(fname));
    } catch (std::exception& e) {
        QMessageBox::critical(this, "Open", QString::fromStdString(e.what()));
    }
}


void FmMain::doOpen()
{
    filedlg::Filters filters
      { { L"UTranslator files", L"*.uorig *.ufull" }, { L"All files", L"*" } };
    auto fname = filedlg::open(
                this, nullptr, filters, {},
                filedlg::AddToRecent::YES);
    if (!fname.empty()) {
        openFile(fname);
    }
}


void FmMain::doSave()
{
    if (!project)
        return;
    if (project->fname.empty()) {
        doSaveAs();
    } else {
        acceptCurrObject();
        try {
            project->save();
            config::history.pushFile(project->fname);
        } catch (std::exception& e) {
            QMessageBox::critical(this, "Save problem", QString::fromStdString(e.what()));
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
    html += "<table border='0'>";
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
                if (auto fplace = std::dynamic_pointer_cast<hist::FilePlace>(place)) {
                    openFile(fplace->path());
                }
            }
        }
    }
}
