// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// STL
#include <deque>

// Qt
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTimer>
#include <QShortcut>

// Qt misc
#include "QModels.h"
#include "DblClickSvgWidget.h"

// Libs
#include "u_Qstrings.h"
#include "i_OpenSave.h"

// Translation
#include "TrFinder.h"

// L10n
#include "LocFmt.h"

// Project-local
#include "d_Config.h"
#include "d_Strings.h"
#include "QtDiff.h"
#include "Tools/QstrObject.h"

// UI forms
#include "FmNew.h"
#include "FmDisambigPair.h"
#include "FmFileFormat.h"
#include "FmFind.h"
#include "FmProjectSettings.h"
#include "FmMessage.h"
#include "FmTrash.h"
#include "Tools/FmDecoder.h"
#include "Tools/FmTranslateWithOriginal.h"
#include "Tools/FmExtractOriginal.h"
#include "Tools/FmSwitchOriginalAndTranslation.h"


///// FmMain ///////////////////////////////////////////////////////////////////


FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);
    config::history.setListener(this);

    ui->wiFind->close();
    dismissUpdateInfo();
    retrieveVersion();
    loadBugImages();

    // Bugs
    timerBug = std::make_unique<QTimer>();
      timerBug->setInterval(600);
      timerBug->setSingleShot(true);
    connect(timerBug.get(), &QTimer::timeout, this, &This::bugTicked);
    showBugs({});

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
    connect(ui->edOrigFilePath, &QLineEdit::textEdited, this, &This::tempModify);
    connect(ui->edTranslFilePath, &QLineEdit::textEdited, this, &This::tempModify);
    connect(ui->memoOriginal, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->chkIdless, &QCheckBox::clicked, this, &This::tempModify);
    connect(ui->memoTranslation, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->memoComment, &QPlainTextEdit::textChanged, this, &This::tempModify);
    connect(ui->btFileFormat, &QPushButton::clicked, this, &This::editFileFormat);

    // Signals/slots: search
    connect(ui->wiFind, &WiFind::closed, this, &This::searchClosed);
    connect(ui->wiFind, &WiFind::repeated, this, &This::goSearchAgain);
    connect(ui->wiFind, &WiFind::indexChanged, this, &This::searchChanged);

    // Signals/slots: update
    connect(ui->btUpdateDismiss,  &QAbstractButton::clicked, this, &This::dismissUpdateInfo);
    connect(ui->btUpdateDismiss2, &QAbstractButton::clicked, this, &This::dismissUpdateInfo);
    connect(ui->btUpdateInfo, &QAbstractButton::clicked, this, &This::showUpdateInfo);
    connect(ui->btUpdateFind, &QAbstractButton::clicked, this, &This::goChangedToday);

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
    connect(ui->acUpdateData, &QAction::triggered, this, &This::doUpdateData);
    connect(ui->acProjectProps, &QAction::triggered, this, &This::doProjectProps);    
    connect(ui->acStats, &QAction::triggered, this, &This::doProjectStats);
    connect(ui->acStartingScreen, &QAction::triggered, this, &This::goToggleStart);
    // Original
    connect(ui->acAddHostedFile, &QAction::triggered, this, &This::addHostedFile);
    connect(ui->acAddHostedGroup, &QAction::triggered, this, &This::addHostedGroup);
    connect(ui->acAddSyncGroup, &QAction::triggered, this, &This::addSyncGroup);
    connect(ui->acAddText, &QAction::triggered, this, &This::addText);
    connect(ui->acDelete, &QAction::triggered, this, &This::doDelete);
    connect(ui->acClone, &QAction::triggered, this, &This::doClone);
    connect(ui->acMoveUp, &QAction::triggered, this, &This::doMoveUp);
    connect(ui->acMoveDown, &QAction::triggered, this, &This::doMoveDown);
    connect(ui->acClearGroup, &QAction::triggered, this, &This::clearGroup);
    connect(ui->acLoadTexts, &QAction::triggered, this, &This::doLoadText);
    // Edit
    connect(ui->acAcceptChanges, &QAction::triggered, this, &This::acceptCurrObjectAll);
    connect(ui->acRevertChanges, &QAction::triggered, this, &This::revertCurrObject);
    connect(ui->acMarkAttention, &QAction::triggered, this, &This::markAttentionCurrObject);
    connect(ui->acDecoder, &QAction::triggered, this, &This::runDecoder);
    connect(ui->acTrash, &QAction::triggered, this, &This::runTrash);
        // Edit — double clicks
        connect(imgBug.origChanged, &DblClickSvgWidget::doubleClicked, this, &This::acceptCurrObjectOrigChanged);
        connect(imgBug.emptyTransl, &DblClickSvgWidget::doubleClicked, this, &This::acceptCurrObjectEmptyTransl);
        connect(imgBug.attention  , &DblClickSvgWidget::doubleClicked, this, &This::removeAttentionCurrObject);
        // Edit — special shortcuts
        shMarkAttention = new QShortcut(ui->acMarkAttention->shortcut(), this);
        connect(shMarkAttention, &QShortcut::activated, this, &This::markAttentionCurrObject);
    // Go
    connect(ui->acGoBack, &QAction::triggered, this, &This::goBack);
    connect(ui->acGoNext, &QAction::triggered, this, &This::goNext);
    connect(ui->acGoUp, &QAction::triggered, this, &This::goUp);
    connect(ui->acGoFindNext, &QAction::triggered, ui->wiFind, &WiFind::goNext);
    connect(ui->acGoFindPrev, &QAction::triggered, ui->wiFind, &WiFind::goBack);
    connect(ui->acGoCloseSearch, &QAction::triggered, ui->wiFind, &WiFind::close);
    connect(ui->acGoSearchAgain, &QAction::triggered, this, &This::goSearchAgain);
    setSearchAction(ui->acGoFind, &This::goFind);
    setSearchAction(ui->acFindWarningsAll, &This::goAllWarnings);
    setSearchAction(ui->acFindWarningsChangedUntransl, &This::goChangedUntransl);
    setSearchAction(ui->acFindWarningsChangedOnly, &This::goChangedOnly);
    setSearchAction(ui->acFindWarningsUntranslatedOnly, &This::goUntranslOnly);
    setSearchAction(ui->acFindWarningsAttention, &This::goAttention);
    setSearchAction(ui->acFindChangedToday, &This::goChangedToday);
    setSearchAction(ui->acFindSpecialMismatchNumber, &This::goMismatchNumber);
    setSearchAction(ui->acFindSpecialCommentedByAuthor, &This::goCommentedByAuthor);
    setSearchAction(ui->acFindSpecialCommentedByTranslator, &This::goCommentedByTranslator);
    // Tools
    connect(ui->acTranslateWithOriginal, &QAction::triggered, this, &This::translateWithOriginal);
    connect(ui->acExtractOriginal, &QAction::triggered, this, &This::extractOriginal);
    connect(ui->acSwitchOriginalAndTranslation, &QAction::triggered, this, &This::switchOriginalAndTranslation);
    connect(ui->acResetKnownOriginals, &QAction::triggered, this, &This::resetKnownOriginals);

    // Shortcuts
    shAddGroup = addBadShortcut(ui->acAddHostedGroup);
    shAddText  = addBadShortcut(ui->acAddText);

    // Unused parts
    ui->grpCompatId->hide();

    // Reference
    ui->grpReference->hide();

    setEditorsEnabled(false);   // while no project, let it be false
    updateCaption();

    goStart();
}

FmMain::~FmMain()
{
    delete ui;
}


DblClickSvgWidget* FmMain::loadBugWidget(
        const char* path, const char* description)
{
    auto parentWi = ui->grpBugs;
    auto wi = new DblClickSvgWidget(parentWi);
    wi->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    wi->setMinimumSize(32, 32);
    wi->setToolTipDuration(60000);
    wi->setToolTip(description);
    wi->load(QString(path));
    auto layout = qobject_cast<QHBoxLayout*>(parentWi->layout());
    auto nWidgets = layout->count();
        // minus 1: the last is spacer
    layout->insertWidget(nWidgets - 1, wi);
    wi->hide();
    return wi;
}


void FmMain::loadBugImages()
{
    // The order of images is programed HERE!
    /// @todo [L10n] strings hardcoded here
    // OK
    imgBug.ok = loadBugWidget(":Discrep/ok.svg",
            "<b>No discrepancies</b>" "\n"
            "<p>UTranslator found no problems."
            "<p><b>Do not</b> chase this sign: "
                "sometimes extra spaces and line breaks are your intentions.");
    // Critical
    imgBug.origChanged = loadBugWidget(":/Discrep/review.svg",
            "<b>Review needed</b>" "\n"
            "<p>The original text was changed. "
                "Please check whether existing translation still works, or retranslate if needed."
            "<p>If everything’s OK, press “Edit → Accept changes”, "
                "or double-click this icon.");
    imgBug.emptyTransl = loadBugWidget(":/Discrep/empty.svg",
            "<b>Empty translation</b>" "\n"
            "<p>If empty string is your actual translation, "
                "press “Edit → Accept changes”, or double-click this icon.");
    imgBug.attention = loadBugWidget(":/Discrep/attention.svg",
            "<b>Bad text</b>" "\n"
            "<p>Someone marked the text as bad to draw the attention of other editors. "
                "Maybe he/she wrote something in comments or development forum."
            "<p>If everything’s OK, press “Edit → Mark as bad”, "
                "or double-click this icon.");
    // Important
    imgBug.mojibake = loadBugWidget(":/Discrep/mojibake.svg",
            "<b>Mojibake</b>" "\n"
            "<p>One of fields was not decoded properly. "
                "Check it twice before accepting.");
    // Information
    imgBug.emptyOrig = loadBugWidget(":/Discrep/empty_b.svg",
            "<b>Empty original</b>" "\n"
            "<p>The original string is completely empty. "
                "Beware, concatenating strings is usually bad, "
                    "substitution marks are better, "
                    "and self-sufficient strings are the best."
            "<p>Horrible: British + Grand Prix"   TAG_BR
                "Poor: British + Grand Prix + ⌀"  TAG_BR
                "Better: %s Grand Prix"           TAG_BR
                "Best: British Grand Prix");
    imgBug.invisible = loadBugWidget(":/Discrep/invisible.svg",
            "<b>Whitespace only</b>" "\n"
            "<p>The string consists of invisible characters, "
                    "such as line breaks and spaces.");
    imgBug.multiline = loadBugWidget(":/Discrep/multiline.svg",
            "<b>Multiline translation</b>" "\n"
            "<p>Translation is multiline, original isn’t. "
                "Check for yourself whether it’s OK.");
    imgBug.space.addBeg = loadBugWidget(":/Discrep/space_add_beg.svg",
            "<b>Added heading spaces</b>" "\n"
            "<p>Original has no initial spaces, translation has. "
                STR_SPACE_MAY_LEAD);
    imgBug.space.delBeg = loadBugWidget(":/Discrep/space_del_beg.svg",
            "<b>Removed heading spaces</b>" "\n"
            "<p>Original has initial spaces, translation hasn’t. "
                STR_SPACE_MAY_LEAD);
    imgBug.space.addEnd = loadBugWidget(":/Discrep/space_add_end.svg",
            "<b>Added trailing spaces</b>" "\n"
            "<p>Original has no final spaces, translation has. "
                STR_SPACE_MAY_LEAD);
    imgBug.space.delEnd = loadBugWidget(":/Discrep/space_del_end.svg",
            "<b>Removed trailing spaces</b>" "\n"
            "<p>Original has final spaces, translation hasn’t. "
                STR_SPACE_MAY_LEAD);
}


QShortcut* FmMain::addBadShortcut(QAction* action)
{
    auto r = new QShortcut(action->shortcut(), this);
    connect(r, &QShortcut::activated, this, &This::badShortcut);
    return r;
}


void FmMain::setSearchAction(QAction* action, void (FmMain::* func)())
{
    connect(action, &QAction::triggered, this, func);
    searchActions.push_back(action);
}


void FmMain::retrieveVersion()
{
    auto version = QApplication::applicationVersion();
        // Count “.” chars
    int nDots = 0;
    for (auto c : version)
        if (c == '.')
            ++nDots;
    // Remove '.0' if there will be dots remaining
    while (nDots > 1 && version.endsWith(".0")) {
        version.resize(version.length() - 2);
        --nDots;
    }
    ui->lbVersion->setText("v" + version);
}


void FmMain::showMessageOverTree(std::u8string_view msg)
{
    fmMessage.ensure(this).showOverWidget(str::toQ(msg), ui->treeStrings);
}


void FmMain::badShortcut()
{
    showMessageOverTree(u8"Cannot edit original");
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
        if (auto currentPrj = currentObj->vproject()) {
            if (formerObj->vproject() == currentPrj)
                acceptObject(*formerObj, {});
            setEditorsEnabled(true);
            loadObject(*currentObj);
            return;
        }
    }
    // Otherwise
    setEditorsEnabled(false);
    reenableOnSelect(nullptr);
}


void FmMain::setMemo(QWidget* wi, QPlainTextEdit* memo,
                     std::u8string_view placeholder,
                     std::u32string_view y)
{
    wi->setEnabled(true);
    isChangingProgrammatically = true;
    memo->setPlaceholderText(str::toQ(placeholder));
    memo->setPlainText(str::toQ(y));
    isChangingProgrammatically = false;
}

void FmMain::banMemo(QWidget* wi, QPlainTextEdit* memo)
{
    wi->setEnabled(false);
    isChangingProgrammatically = true;
    memo->setPlaceholderText({});
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
            auto shownCmt = cmt->importersOrAuthors();
            if (!shownCmt.empty()) {
                itsText += "<dd>";                
                auto text = str::toQ(shownCmt).toHtmlEscaped();
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


void FmMain::clearReference()
{
    ui->memoReference->setEnabled(false);
    ui->memoReference->clear();
}


void FmMain::loadObject(tr::UiObject& obj)
{
    if (search.result) {
        auto sp = obj.selfUi();
        if (auto index = search.result->find(sp); index != NOT_FOUND) {
            ui->wiFind->setIndexQuietly(index);
        }
    }

    // Reference
    ui->grpReference->setVisible(obj.vproject()->prjInfo().hasReference());

    bugCache.copyFrom(obj);

    auto id = str::toQ(obj.idColumn());
    ui->edId->setText(id);
    // File/Original/translation
    if (auto file = dynamic_cast<tr::File*>(&obj)) {
        // FILE
        ui->stackOriginal->setCurrentWidget(ui->pageFile);
        ui->chkIdless->setChecked(file->info.isIdless);
        ui->edOrigFilePath->setPlaceholderText(ui->edId->text());
        auto origPath = str::toQ(file->info.origPath.wstring());
        ui->edOrigFilePath->setText(origPath);
        /// @todo [repeat] the file itself should have this logic
        ui->edTranslFilePath->setPlaceholderText(
                    !origPath.isEmpty() ? origPath : id);
        ui->edTranslFilePath->setText(str::toQ(file->info.translPath.wstring()));
        const bool canAddFiles = project->info.canAddFiles();
        const bool canEditOriginal = project->info.canEditOriginal();
        const bool isTranslation = project->info.isTranslation();
        ui->wiFileLo->setEnabled(canAddFiles);
        ui->wiId->setEnabled(canAddFiles);
        ui->wiOrigName->setVisible(canEditOriginal);
        ui->wiTranslName->setVisible(isTranslation);
        clearReference();
    } else if (bugCache.hasTranslatable) {
        // ORIGINAL, mutually exclusive with fileInfo
        ui->wiId->setEnabled(project->info.canEditOriginal());
        if (bugCache.isProjectOriginal) {
            ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
            setMemo(ui->grpOriginal, ui->memoOriginal, {}, bugCache.original);
        } else {
            ui->stackOriginal->setCurrentWidget(ui->pageUneditableOriginal);
            auto doc = ui->richedOriginal->document();
            doc->clear();
            QTextCursor cursor(doc);
            if (bugCache.knownOriginal) {
                qdif::write2(cursor,
                             *bugCache.knownOriginal,
                             bugCache.original,
                             "<font size='-1' style='color:gray;'>== Used to be ==</font>");
            } else {
                qdif::write1(cursor, bugCache.original);
            }
        }
        if (bugCache.reference) {
            ui->memoReference->setEnabled(true);
            ui->memoReference->setPlainText(str::toQ(*bugCache.reference));
        } else {
            ui->memoReference->setEnabled(false);
            ui->memoReference->setPlainText(STR_UNTRANSLATED);
        }
        setMemo(ui->grpTranslation, ui->memoTranslation, {}, bugCache.translation);
    } else {
        // GROUP
        ui->wiId->setEnabled(project->info.canEditOriginal());
        ui->stackOriginal->setCurrentWidget(ui->pageOriginal);
        ui->grpOriginal->setEnabled(false);
        ui->grpTranslation->setEnabled(false);
        banMemo(ui->grpOriginal, ui->memoOriginal);
        banMemo(ui->grpTranslation, ui->memoTranslation);
        clearReference();
    }
    // Comment
    if (bugCache.hasComments) {
        if (bugCache.isProjectOriginal) {
            // Bilingual (currently unimplemented):
            // can edit original → author’s comment; cannot → translator’s
            setMemo(ui->grpComment, ui->memoComment, bugCache.comm.importers, bugCache.comm.editable);
        } else {
            setMemo(ui->grpComment, ui->memoComment, {}, bugCache.comm.editable);
        }
    } else {
        banMemo(ui->grpComment, ui->memoComment);
    }
    // Context
    if (bugCache.isProjectOriginal) {
        // If we edit original → load parent
        loadContext(obj.parent().get());
    } else {
        // Otherwise load self!
        loadContext(&obj);
    }

    reenableOnSelect(obj);
    showBugs(bugCache.bugs());
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

    inline std::u32string toText(QString x)
    {
        normalizeEol(x);
        return x.toStdU32String();
    }

    std::u32string toText(QPlainTextEdit* x)
        { return toText(x->toPlainText()); }

}   // anon namespace


void FmMain::uiToCache(tr::BugCache& r)
{
    r = bugCache;
    r.id = toText(ui->edId->text());
    if (r.canEditOriginal()) {
        r.original = toText(ui->memoOriginal);
    }
    if (r.canEditTranslation()) {
        r.translation = toText(ui->memoTranslation);
    }
    if (r.hasComments) {
        r.comm.editable = toText(ui->memoComment);
    }
}


void FmMain::acceptObject(tr::UiObject& obj, Flags<tr::Bug> bugsToRemove)
{
    tr::BugCache newCache;
    uiToCache(newCache);

    newCache.copyTo(obj, bugCache, bugsToRemove);
    if (project) {
        if (project->info.canAddFiles()) {
            obj.setIdless(ui->chkIdless->isChecked(), tr::Modify::YES);
        }
        if (project->info.canEditOriginal()) {
            obj.setOrigPath(ui->edOrigFilePath->text().toStdWString(), tr::Modify::YES);
        }
        if (project->info.isTranslation()) {
            obj.setTranslPath(ui->edTranslFilePath->text().toStdWString(), tr::Modify::YES);
        }
        project->tempRevert();
    }
    obj.stats(tr::StatsMode::SEMICACHED, tr::CascadeDropCache::YES);
    emit treeModel.dataChanged({}, {});

    // “Bugs to remove” is also the sign that we go on editing
    if (bugsToRemove) {
        bugCache = std::move(newCache);
        showBugs(bugCache.bugs());
    }
}


tr::UiObject* FmMain::acceptCurrObject(Flags<tr::Bug> bugsToRemove)
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    acceptObject(*obj, bugsToRemove);
    return obj;
}


tr::UiObject* FmMain::acceptCurrObjectNone()
{
    return acceptCurrObject({});
}


tr::UiObject* FmMain::acceptCurrObjectOrigChanged()
{
    return acceptCurrObject(tr::Bug::TR_ORIG_CHANGED);
}


tr::UiObject* FmMain::acceptCurrObjectEmptyTransl()
{
    return acceptCurrObject(tr::Bug::TR_EMPTY);
}


tr::UiObject* FmMain::acceptCurrObjectAll()
{
    return acceptCurrObject(tr::Bug::ALL_SERIOUS);
}


void FmMain::tempModify()
{
    if (project && !isChangingProgrammatically) {
        project->tempModify();
        windBugTimer();
    }
}


void FmMain::revertCurrObject()
{
    auto index = treeIndex();
    auto obj = treeModel.toObj(index);
    loadObject(*obj);
    if (project)
        project->tempRevert();
}


void FmMain::reenable()
{
    bool isStartVisible = (ui->stackMain->currentWidget() == ui->pageStart);
    bool isMainVisible = (ui->stackMain->currentWidget() == ui->pageMain);
    bool hasProject { project };
    bool isOriginal = (isMainVisible && hasProject
                       && project->info.canEditOriginal());
    bool isNotOriginal = (isMainVisible && hasProject
                       && !project->info.canEditOriginal());
    bool canAddFiles = (isMainVisible && hasProject
                       && project->info.canAddFiles());
    bool canSearch = ui->wiFind->isVisible() && isMainVisible;

    // Starting screen
    ui->btStartEdit->setVisible(hasProject);

    // Menu: File
        // New, Open are always available
    ui->acSave->setEnabled(hasProject);
    ui->acSaveAs->setEnabled(hasProject);
    ui->acProjectProps->setEnabled(hasProject && isMainVisible);
    ui->acUpdateData->setEnabled(hasProject && isMainVisible);
    ui->acStartingScreen->setEnabled(hasProject);
        ui->acStartingScreen->setChecked(hasProject && isStartVisible);
    ui->acStats->setEnabled(hasProject);

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

    // Menu: Original
    shAddGroup->setEnabled(isNotOriginal);
    shAddText->setEnabled(isNotOriginal);

    // Menu: Edit
    ui->acAcceptChanges->setEnabled(isMainVisible);
    ui->acRevertChanges->setEnabled(isMainVisible);
    ui->acTrash->setEnabled(hasProject);

    // Menu: Go
    ui->acGoBack->setEnabled(isMainVisible);
    ui->acGoNext->setEnabled(isMainVisible);
    ui->acGoUp->setEnabled(isMainVisible);
    for (auto v : searchActions)
        v->setEnabled(isMainVisible);
    ui->acGoFindNext->setEnabled(canSearch);
    ui->acGoFindPrev->setEnabled(canSearch);
    ui->acGoCloseSearch->setEnabled(canSearch);
    ui->acGoSearchAgain->setEnabled(canSearch);

    // Menu: Tools
    ui->acTranslateWithOriginal->setEnabled(isMainVisible);
    ui->acExtractOriginal->setEnabled(isMainVisible);
    ui->acSwitchOriginalAndTranslation->setEnabled(isMainVisible);
    ui->acResetKnownOriginals->setEnabled(isMainVisible);

    reenableOnSelect();
}


void FmMain::reenableOnSelect(tr::UiObject* obj)
{
    bool isMainVisible = (ui->stackMain->currentWidget() == ui->pageMain);
    bool isObjVisible = isMainVisible && obj;
    const tr::Translatable* trable = isObjVisible ? obj->translatable() : nullptr;
    bool isTranslatable = trable;
    // Mark attention (bad in UI)
    ui->acMarkAttention->setEnabled(isTranslatable);
    shMarkAttention->setEnabled(isMainVisible && !isTranslatable);
    ui->acMarkAttention->setChecked(isTranslatable && trable->forceAttention);
}


void FmMain::reenableOnSelect()
{
    auto index = treeIndex();
    auto obj = treeModel.toObjOr(index, nullptr);
    reenableOnSelect(obj);
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
    auto fileInfo = obj->inheritedFileInfo();
    switch (editMode) {
    case EditMode::TEXT:
        if (fileInfo && fileInfo->isIdless) {
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
    auto obj = acceptCurrObjectNone();
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
        auto group = treeModel.addGroup(*dis);
        if (group) {    // Have group
            startEditingOrig(group.index, EditMode::GROUP);
        } else {        // No group
            QMessageBox::warning(this,
                        "Add group",
                        "Select some file or group");
        }
    }
}


void FmMain::addSyncGroup()
{
    if (auto dis = disambigGroup(u8"Add synchronized group")) {
        if (!*dis) {
            QMessageBox::warning(this,
                        "Add synchronized group",
                        "Select some file or group");
            return;
        }
        auto fileFormat = loadSetsCache.format.clone();
        auto isOk = fmFileFormat.ensure(this).exec(
                    fileFormat,
                    static_cast<tf::LoadTextsSettings*>(nullptr),
                    &loadSetsCache.syncInfo,
                    tf::ProtoFilter::ALL_IMPORTING);
        if (isOk) {
            loadSetsCache.format = std::move(fileFormat);
            auto filter = loadSetsCache.format->fileFilter();
            filedlg::Filters filters { filter, filedlg::ALL_FILES };
            std::filesystem::path fileName = filedlg::open(
                    this, {}, filters, filter.extension(),
                    filedlg::AddToRecent::NO);

            if (!fileName.empty()) {
                auto group = treeModel.addGroup(*dis);
                if (group) {    // Have group
                    group.subj->sync = {
                        .format = loadSetsCache.format.clone(),
                        .absPath = std::filesystem::weakly_canonical(fileName),
                        .info = loadSetsCache.syncInfo,
                    };
                    try {
                        { auto thing = lockAll(RememberCurrent::NO);
                            group.subj->loadText(*loadSetsCache.format, fileName,
                                            tf::Existing::OVERWRITE);
                            // This group will be expanded
                            group.subj->cache.treeUi.expandState = tr::ExpandState::EXPANDED;
                        }
                        startEditingOrig(group.index, EditMode::GROUP);
                    } catch (const std::exception& e) {
                        QMessageBox::critical(this, "Load texts", QString::fromStdString(e.what()));
                    }
                } // if group
            } // if filename
        } // if isOk
    }
}


void FmMain::addText()
{
    /// @todo [urgent, #70] The chain is:
    ///       disambigGroup() returns concrete VirtualGroup
    ///       VirtualGroup’s add() returns text
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
        auto nTexts = stats.text.nTotal();
        switch (nTexts) {
        case 0:
            if (stats.nGroups == 0)
                message = "Delete empty group?";
                else message = "Delete group w/o texts?";
            break;
        default:
            message = loc::Fmt(u8"Delete {1|one=? text|many=? texts}?")(nTexts).q();
        }

    }
    auto answer = QMessageBox::question(this, "Delete", message);
    if (answer != QMessageBox::Yes)
        return;
    /// @todo [find, #20] limit search instead of complete closing?
    ui->wiFind->close();
    treeModel.extract(obj);
}


void FmMain::modStateChanged(ModState oldState, ModState newState)
{
    if (newState == ModState::UNMOD && oldState == ModState::MOD)
        emit treeModel.dataChanged({}, {});
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
        filters.emplace_back( filedlg::Filter { PAIR_ORIGINAL });
        extension = W(EXT_ORIGINAL);
        break;
    case tr::PrjType::FULL_TRANSL:
        filters.emplace_back( filedlg::Filter { PAIR_TRANSLATION });
        extension = W(EXT_TRANSLATION);
        break;
    }
    filters.emplace_back(filedlg::ALL_FILES);

    acceptCurrObjectNone();
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


namespace {

    enum class EnableExec : unsigned char { NO, YES };

    ///
    /// \brief  Executes something afterwards
    ///    Used in error-handling code, when there are lots of execution paths,
    ///    and we just need to mark whether we execute it.
    ///
    template <class T> class ExecAfter {
    public:
        constexpr ExecAfter(EnableExec aIsEnabled, const T& aBody) noexcept
            : isEnabled(static_cast<bool>(aIsEnabled)), body(aBody) {}
        ~ExecAfter() noexcept(noexcept(body())) {
            if (isEnabled)
                body();
        }
        constexpr void enable() { isEnabled = true; }
        constexpr void disable() { isEnabled = false; }
    private:
        bool isEnabled;
        const T& body;
    };

}   // anon namespace


void FmMain::openFileThrow(std::filesystem::path fname, OpenPlace& rPlace)
{
    rPlace = OpenPlace::PROJECT;
    dismissUpdateInfo();
    auto prj = tr::Project::make();
    prj->load(fname);
    ui->wiFind->close();
    rPlace = OpenPlace::REFERENCE;

    auto whatAfter = [&prj, &fname, this]() {
        plantNewProject(std::move(prj));
        config::history.pushFile(std::move(fname));
    };
    ExecAfter ea(EnableExec::YES, whatAfter);
    prj->updateReference();
    // ExecAfter will exec here!
}

namespace {

    constinit const char* HEAD_PROJECT = "Open";
    constinit const char* HEAD_REFERENCE = "Load reference";

}

void FmMain::handleReferenceError(const char* text)
{
    QMessageBox::warning(this, HEAD_REFERENCE, QString::fromStdString(text));
}


void FmMain::openFile(std::filesystem::path fname)      // by-value + move
{
    OpenPlace openPlace = OpenPlace::PROJECT;
    try {
        openFileThrow(std::move(fname), openPlace);
    } catch (const std::exception& e) {
        switch (openPlace) {
        case OpenPlace::PROJECT:
            QMessageBox::critical(this, HEAD_PROJECT, QString::fromStdString(e.what()));
            break;
        case OpenPlace::REFERENCE:
            handleReferenceError(e.what());
            break;
        }
    }
}

void FmMain::openFileFromHistory(unsigned i)
{
    if (auto place = config::history[i]) {
        if (checkSave("Open")) {
            if (auto fplace = std::dynamic_pointer_cast<hist::FilePlace>(place)) {
                auto openPlace = OpenPlace::PROJECT;
                try {
                    openFileThrow(fplace->path(), openPlace);
                } catch (const std::exception& e) {
                    switch (openPlace) {
                    case OpenPlace::PROJECT: {
                            auto msg = QString::fromStdString(e.what()) + "\n" "Delete from history?";
                            auto result = QMessageBox::critical(this, "Open", msg,
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
                            if (result == QMessageBox::Yes) {
                                config::history.erase(i);
                            }
                        } break;
                    case OpenPlace::REFERENCE:
                        handleReferenceError(e.what());
                        break;
                    }
                }
            }
        }
    }
}

void FmMain::doOpen()
{
    if (!checkSave("Open"))
        return;
    filedlg::Filters filters
      { FILTER_UTRANSL, filedlg::ALL_FILES };
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
        acceptCurrObjectNone();
        dismissUpdateInfo();
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
            openFileFromHistory(i);
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
                    static_cast<tf::SyncInfo*>(nullptr),
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


void FmMain::findBy(std::unique_ptr<tr::FindCriterion> crit)
{
    ts::Finder finder(*crit);
    project->traverse(finder, tr::WalkOrder::EXACT, tr::EnterMe::NO);
    plantSearchResult(std::move(crit), finder.give());
}

void FmMain::goFind()
{
    if (auto opts = fmFind.ensure(this).exec(project->info.type)) {
        auto uopts = std::make_unique<FindOptions>(*opts);
        findBy(std::move(uopts));
    }
}


void FmMain::goAllWarnings()
{
    auto cond = std::make_unique<ts::CritWarning>(project);
    findBy(std::move(cond));
}


void FmMain::goChangedUntransl()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Changed original / untranslated", STR_FIND_TRANSL);
    } else {
        auto cond = std::make_unique<ts::CritChangedUntransl>(project);
        findBy(std::move(cond));
    }
}


void FmMain::goChangedOnly()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Changed original only", STR_FIND_TRANSL);
    } else {
        auto cond = std::make_unique<ts::CritChangedOnly>(project);
        findBy(std::move(cond));
    }
}


void FmMain::goUntranslOnly()
{
    if (!project->info.isFullTranslation()) {
        QMessageBox::information(this, "Untranslated", STR_FIND_FULL_TRANSL);
    } else {
        auto cond = std::make_unique<ts::CritUntranslated>(project);
        findBy(std::move(cond));
    }
}


void FmMain::goAttention()
{
    auto cond = std::make_unique<ts::CritAttention>();
    findBy(std::move(cond));
}


void FmMain::goChangedToday()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Changed/added today", STR_FIND_TRANSL);
    } else {
        auto cond = std::make_unique<ts::CritChangedToday>(project);
        findBy(std::move(cond));
    }
}


void FmMain::goMismatchNumber()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Mismatching # of lines",
                    "This criterion works for bilinguals/translations only, "
                            "and will find nothing in originals.");
    } else {
        auto cond = std::make_unique<ts::CritMismatchNumber>();
        findBy(std::move(cond));
    }
}


void FmMain::goCommentedByAuthor()
{
    auto cond = std::make_unique<ts::CritCommentedByAuthor>();
    findBy(std::move(cond));
}


void FmMain::goCommentedByTranslator()
{
    if (!project->info.isTranslationCommentable()) {
        QMessageBox::information(this, "Commented by translator",
                    "This criterion works for translations only, "
                            "and will find nothing in originals/bilinguals.");
    } else {
        auto cond = std::make_unique<ts::CritCommentedByTranslator>();
        findBy(std::move(cond));
    }
}


void FmMain::goSearchAgain()
{
    findBy(std::move(search.criterion));
}


void FmMain::plantSearchResult(
        std::unique_ptr<tr::FindCriterion> crit,
        std::unique_ptr<ts::Result> x)
{
    if (!x || x->isEmpty()) {
        ui->wiFind->close();    // automatically calls searchClosed and reenables
        QMessageBox::information(this, "Find", "Not found.");
    } else {
        search.criterion = std::move(crit);
        search.result = std::move(x);
            // will not reenable for now
        /// @todo [L10n] Search criterion’s header will remain untranslated
        ui->wiFind->startSearch(
                    str::toQ(search.criterion->caption()), search.result->size());
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
        /// @todo [find, #20] limit search??
        ui->wiFind->close();
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
    auto fileInfo = obj->inheritedFileInfo();
    if (!selectedGroup || !fileInfo) {
        QMessageBox::warning(this, "Load texts", "Please select something!");
        return;
    }

    // Get current file
    CloningUptr<tf::FileFormat> fileFormat;
    if (fileInfo.get() != loadSetsCache.fileKey) {
        if (fileInfo->format)
            fileFormat = fileInfo->format->clone();
    }
    if (!fileFormat && loadSetsCache.format) {
        fileFormat = loadSetsCache.format->clone();
    }

    if (fmFileFormat.ensure(this).exec(
                fileFormat, &loadSetsCache.text,
                static_cast<tf::SyncInfo*>(nullptr),
                tf::ProtoFilter::ALL_IMPORTING)) {
        // Save as soon as we chose smth, even if we press Cancel afterwards
        loadSetsCache.format = std::move(fileFormat);
        loadSetsCache.fileKey = fileInfo.get();
        auto filter = loadSetsCache.format->fileFilter();
        filedlg::Filters filters { filter, filedlg::ALL_FILES };
        std::filesystem::path fileName = filedlg::open(
                this, L"Load texts", filters, filter.extension(),
                filedlg::AddToRecent::NO);

        if (!fileName.empty()) {
            std::shared_ptr<tr::VirtualGroup> destGroup;
            switch (loadSetsCache.text.loadTo) {
            case tf::LoadTo::ROOT:
                /// @todo [bad, #70] dynamic_cast here
                /// @todo [bad, #70] Anyway UiObject’s API is too simple, and w/o it
                ///        there are troubles with orter APIs
                destGroup = dynamic_cast<tr::Traversable*>(obj)->file();
                break;
            case tf::LoadTo::SELECTED:
                destGroup = selectedGroup;
                break;
            }

            try {
                auto thing = lockAll(RememberCurrent::YES);
                destGroup->loadText(*loadSetsCache.format, fileName,
                                    loadSetsCache.text.existing);
                // This group will be expanded
                destGroup->cache.treeUi.expandState = tr::ExpandState::EXPANDED;
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
    search.result.reset();
    search.criterion.reset();
    reenable();
}


void FmMain::searchChanged(size_t index)
{
    if (search.result) {
        if (auto obj = search.result->at(index)) {
            if (auto index = treeModel.toIndex(obj, 0);
                    index.isValid()) {
                ui->treeStrings->setCurrentIndex(index);
            }
        }
    }
}


void FmMain::dismissUpdateInfo()
{
    ui->stackUpdate->hide();
}


void FmMain::reflectUpdateInfo()
{
    if (project && updateInfo.hasSmth()) {
        char c[120];
        if (project->info.isTranslation()) {
            snprintf(c, std::size(c), "Changes found: add %llu, del %llu/%llu, chg %llu/%llu",
                     static_cast<unsigned long long>(updateInfo.nAdded),
                     static_cast<unsigned long long>(updateInfo.deleted.nTranslated),
                     static_cast<unsigned long long>(updateInfo.deleted.nUntranslated),
                     static_cast<unsigned long long>(updateInfo.changed.nTranslated),
                     static_cast<unsigned long long>(updateInfo.changed.nUntranslated));
        } else {
            snprintf(c, std::size(c), "Changes found: add %llu, del %llu, chg %llu",
                     static_cast<unsigned long long>(updateInfo.nAdded),
                     static_cast<unsigned long long>(updateInfo.deleted.nTotal()),
                     static_cast<unsigned long long>(updateInfo.changed.nTotal()));
        }
        ui->lbUpdate->setText(c);
        ui->stackUpdate->setCurrentWidget(ui->pageUpdateChanged);
    } else {
        ui->stackUpdate->setCurrentWidget(ui->pageUpdateUntouched);
    }
    ui->stackUpdate->show();
}



void FmMain::doProjectProps()
{
    if (!project)
        return;
    if (fmProjectSettings.ensure(this).exec(project->info)) {
        project->modify();
        if (auto tmp = treeModel.checkProps()) {
            auto lk = treeModel.lock(ui->treeStrings, RememberCurrent::YES);
            // do nothing under that lock for now,
            // and lk will build meanings once again
        }
    }
}


void FmMain::updateSyncGroups()
{
    auto syncGroups = project->syncGroups();
    if (syncGroups.empty()) {
        QMessageBox::information(this, "Update data",
                "This original has no synchronized groups. Nothing to update.");
    } else {
        ui->wiFind->close();
        std::shared_ptr<tr::Group> thrownGroup;
        updateInfo = tr::UpdateInfo::ZERO;
        try {
            auto lk = lockAll(RememberCurrent::YES);
            for (const auto& sg : syncGroups) {
                thrownGroup = sg;  // throws exception → know which group
                updateInfo += sg->updateData();
            }
            reflectUpdateInfo();
        } catch (std::exception& e) {
            reflectUpdateInfo();
            QString text;
            if (thrownGroup) {
                text += u8"While updating “";
                text += str::toQ(thrownGroup->sync.absPath.filename().u8string());
                text += u8"”:\n";
            }
            text += e.what();
            QMessageBox::critical(this, "Update data", text);
        }
    }
}


namespace {
    enum class UpdatePlace : unsigned char { ORIGINAL, REFERENCE };
}   // anon namespace


void FmMain::updateOriginal()
{
    ui->wiFind->close();
    auto place = UpdatePlace::ORIGINAL;
    try {
        auto whatExec = [this]{ reflectUpdateInfo(); };
        ExecAfter ex(EnableExec::NO, whatExec);
        { auto lk = lockAll(RememberCurrent::YES);
            updateInfo = project->updateData(tr::TrashMode::FILL);
            ex.enable();
            place = UpdatePlace::REFERENCE;
            project->updateReference();
        }
    } catch (std::exception& e) {
        switch (place) {
        case UpdatePlace::ORIGINAL:
            QMessageBox::critical(this, "Update original", e.what());
            break;
        case UpdatePlace::REFERENCE:
            handleReferenceError(e.what());
            break;
        }
    }
}


void FmMain::doUpdateData()
{
    if (!project)
        return;
    switch (project->info.type) {
    /// @todo [bilingual, #28] in synced groups bilinguals WILL have knownOriginal’s
    case tr::PrjType::ORIGINAL:
        updateSyncGroups();
        break;
    case tr::PrjType::FULL_TRANSL:
        updateOriginal();
        break;
    }
}


namespace {

    class ShowNone
    {
    public:
        ShowNone(Flags<tr::Bug> x) : bugs(x) {}
        void showIf(QWidget* what, bool condition);
        void showIfBug(QWidget* what, tr::Bug bug);
        bool isNoneShown() const { return !wasSmthShown; }
    private:
        Flags<tr::Bug> bugs;
        bool wasSmthShown = false;
    };

    void ShowNone::showIf(QWidget* what, bool condition)
    {
        what->setVisible(condition);
        wasSmthShown |= condition;
    }

    void ShowNone::showIfBug(QWidget* what, tr::Bug bug)
    {
        showIf(what, bugs.have(bug));
    }

}   // anon namespace


void FmMain::showBugs(Flags<tr::Bug> x)
{
    stopBugTimer();
    ShowNone sh(x);
    // Problems
    sh.showIfBug(imgBug.emptyTransl , tr::Bug::TR_EMPTY );
    sh.showIfBug(imgBug.origChanged , tr::Bug::TR_ORIG_CHANGED);
    sh.showIfBug(imgBug.attention   , tr::Bug::COM_ATTENTION);
    // Warnings
    sh.showIfBug(imgBug.mojibake    , tr::Bug::COM_MOJIBAKE);
    // Info
    sh.showIfBug(imgBug.emptyOrig   , tr::Bug::OR_EMPTY);
    sh.showIfBug(imgBug.invisible   , tr::Bug::COM_INVISIBLE);
    sh.showIfBug(imgBug.multiline   , tr::Bug::TR_MULTILINE);
    sh.showIfBug(imgBug.space.addBeg, tr::Bug::TR_SPACE_HEAD_ADD);
    sh.showIfBug(imgBug.space.delBeg, tr::Bug::TR_SPACE_HEAD_DEL);
    sh.showIfBug(imgBug.space.addEnd, tr::Bug::TR_SPACE_TAIL_ADD);
    sh.showIfBug(imgBug.space.delEnd, tr::Bug::TR_SPACE_TAIL_DEL);
    imgBug.ok->setVisible(sh.isNoneShown());
}


PrjTreeModel::LockAll FmMain::lockAll(RememberCurrent rem)
{
    acceptCurrObjectNone();
    return treeModel.lock(ui->treeStrings, rem);
}


namespace {

    void sep(QString& text)
    {
        if (!text.isEmpty())
            text += '\n';
    }

}   // anon namespace


void FmMain::showUpdateInfo()
{
    QString text;
    text += loc::Fmt(u8"Strings added: {}")(updateInfo.nAdded).q();
    sep(text);
    text += loc::Fmt(u8"Strings removed: {} translated, {} untranslated")
                (updateInfo.deleted.nTranslated, updateInfo.deleted.nUntranslated).q();
    sep(text);
    text += loc::Fmt(u8"Strings changed: {} translated, {} untranslated")
                (updateInfo.changed.nTranslated, updateInfo.changed.nUntranslated).q();

    QMessageBox::information(this, "Update info", text);
}


void FmMain::windBugTimer()
{
    timerBug->stop();
    timerBug->start();
}


void FmMain::stopBugTimer()
{
    timerBug->stop();
}


void FmMain::bugTicked()
{
    QModelIndex index = ui->treeStrings->currentIndex();
    auto obj = treeModel.toObj(index);
    auto lkBug = bugCache.obj.lock();
    if (obj == lkBug.get()) {
        tr::BugCache tmp;
        uiToCache(tmp);
        showBugs(tmp.bugs());
    }
}


void FmMain::extractOriginal()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Extract original",
                    STR_NEED_BILINGUAL_TRANSLATION);
        return;
    }
    if (auto sets = fmExtractOriginal.ensure(this).exec(0)) {
        auto lk = lockAll(RememberCurrent::YES);
        tr::extractOriginal(*project, *sets);
    }
}


void FmMain::switchOriginalAndTranslation()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Switch original and translation",
                    STR_NEED_BILINGUAL_TRANSLATION);
        return;
    }
    if (auto sets = fmSwitchOriginalAndTranslation.ensure(this).exec(*project)) {
        auto lk = lockAll(RememberCurrent::YES);
        tr::switchOriginalAndTranslation(*project, *sets);
    }
}


void FmMain::resetKnownOriginals()
{
    if (QMessageBox::question(this,
            "Reset known originals",
            "You will get rid of all warnings about changed original, whether true or erroneous." "\n"
                "For example, you may do this due to changed original language, "
                              "or massive grammar checks in original." "\n"
                "Do you really want to do this?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) == QMessageBox::Yes) {
        auto lk = lockAll(RememberCurrent::YES);
        tr::resetKnownOriginal(*project);
    }
}


void FmMain::translateWithOriginal()
{
    if (!project->info.isTranslation()) {
        QMessageBox::information(this, "Translate with original",
                    STR_NEED_BILINGUAL_TRANSLATION);
        return;
    }
    filedlg::Filters filters { FILTER_TRANSLATABLE, filedlg::ALL_FILES };
    std::filesystem::path fileName = filedlg::open(
            this, L"Translate with original", filters, WEXT_ORIGINAL,
            filedlg::AddToRecent::NO);
    if (!fileName.empty()) {
        auto sets = fmTranslateWithOriginal.ensure(this).exec(0);
        if (!sets)
            return;
        sets->origPath = fileName;
        try {
            tr::translateWithOriginal(*project, *sets);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Translate with original", QString::fromStdString(e.what()));
        }
    }
}


void FmMain::markAttentionCurrObjectEx(FuncBoolBool x)
{
    auto index = treeIndex();
    auto obj = treeModel.toObjOr(index, nullptr);
    if (!obj) {
        showMessageOverTree(u8"Nothing is selected");
        return;
    }
    auto trable = obj->translatable();
    if (!trable) {
        showMessageOverTree(u8"This object is not a text");
        return;
    }
    auto newV = x(trable->forceAttention);
    if (newV != trable->forceAttention) {
        trable->forceAttention = newV;
        obj->doModify(tr::Mch::META);
    }
        // Let it be YES for now (cascade is for children, and they are absent)
    obj->stats(tr::StatsMode::DIRECT, tr::CascadeDropCache::YES);
    reenableOnSelect(obj);
    showBugs(bugCache.bugs());
    emit treeModel.dataChanged({}, {});
}


void FmMain::markAttentionCurrObject()
{
    markAttentionCurrObjectEx([](bool x) { return !x; });
}


void FmMain::removeAttentionCurrObject()
{
    markAttentionCurrObjectEx([](bool) { return false; });
}


namespace {

    constexpr std::string_view TMPL_TRANSLATION =
        "{1}: {2|one=? string|many=? strings}, "
             "{3|one=? char|many=? chars} original, "
             "{4|one=? char|many=? chars} translation";
    constexpr std::string_view TMPL_UNTRANSLATED =
        "{1}: {2|one=? string|many=? strings}, "
             "{3|one=? char|many=? chars} original";
    constexpr std::string_view TMPL_ORIGINAL =
        "{1}: {2|one=? string|many=? strings}, "
             "{3|one=? char|many=? chars}";

    void appendLine(
            QString& text,
            tr::BigStats::LittleBigStats& part,
            std::string_view name,
            std::string_view tmpl)
    {
        if (!text.isEmpty())
            text += '\n';
        text += loc::FmtL(tmpl)
                        (name)(part.nStrings)(part.nCpsOrig)(part.nCpsTransl).q();
    }

}   // anon namespace


void FmMain::doProjectStats()
{
    if (!project)
        return;
    auto stats = project->bigStats();
    QString text;
    auto& info = project->info;
    if (info.isTranslation()) {
        appendLine(text, stats.all,      "All",          TMPL_TRANSLATION);
        appendLine(text, stats.untransl, "Untranslated", TMPL_UNTRANSLATED);
        appendLine(text, stats.dubious,  "Dubious",      TMPL_TRANSLATION);
        appendLine(text, stats.transl,   "Translated",   TMPL_TRANSLATION);
    } else {
        appendLine(text, stats.all,      "All",          TMPL_ORIGINAL);
        appendLine(text, stats.untransl, "Normal",       TMPL_ORIGINAL);
        appendLine(text, stats.dubious,  "Dubious",      TMPL_ORIGINAL);
    }
    QMessageBox::information(this, "Statistics", text);
}


void FmMain::runTrash()
{
    if (!project)
        return;
    std::unique_ptr<QstrObject> obj;
    TrashChannel channel = TrashChannel::NOMATTER;
    if (ui->memoOriginal->hasFocus()) {
        obj = std::make_unique<MemoObject>(ui->memoOriginal);
        channel = TrashChannel::ORIGINAL;
    } else if (ui->memoTranslation->hasFocus()) {
        obj = std::make_unique<MemoObject>(ui->memoTranslation);
        channel = TrashChannel::TRANSLATION;
    }
    fmTrash.ensure(this).exec(project->trash, obj.get(), channel);
}
