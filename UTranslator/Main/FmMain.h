#pragma once

// Qt
#include <QMainWindow>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
// Strange bug here
#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wattributes"
#endif
    #include <QUrl>
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#endif

// Libs
#include "u_Uptr.h"

// Project
// No one uses FmMain → you can import EVERYTHING
#include "TrProject.h"
#include "TrBugs.h"

// Project-local
#include "History.h"

// Main’s parts
#include "PrjTreeModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class FmNew;
class FmDisambigPair;
class FmDecoder;
class FmFileFormat;
class FmFind;
class FmProjectSettings;
class FmExtractOriginal;
class FmSwitchOriginalAndTranslation;
class FmTranslateWithOriginal;
class FmMessage;
class FmTrash;
class QPlainTextEdit;
class QTreeView;
class QShortcut;
class DblClickSvgWidget;

namespace ts {
    class Result;
}

enum class OpenPlace : unsigned char { PROJECT, REFERENCE };

using FuncBoolBool = bool (*)(bool);

class FmMain :
        public QMainWindow,
        public ModListener,
        public hist::Listener
{
    Q_OBJECT
    using Super = QMainWindow;
    using This = FmMain;
public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

    // ModListener
    void modStateChanged(ModState oldState, ModState newState) override;
    // hist::Listener
    void historyChanged() override;
private slots:
    // Tree etc.
    void treeCurrentChanged(
            const QModelIndex& current, const QModelIndex& former);
    void tempModify();
    void editFileFormat();
    void dismissUpdateInfo();
    void searchClosed();
    void searchChanged(size_t index);
    void showUpdateInfo();

    // Bugs
    void stopBugTimer();
    void windBugTimer();
    void bugTicked();

    // Starting screen
    void goEdit();
    void startLinkClicked(QUrl url);
    // Menu: File
    void doNew();
    void doOpen();
    bool doSave();
    bool doSaveAs();
    void goToggleStart();
    void goStart();
    void doMoveUp();
    void doMoveDown();
    void doProjectProps();
    void doUpdateData();
    void doProjectStats();
    // Menu: Edit
    /// accept curr object, no bugs gagged
    tr::UiObject* acceptCurrObjectNone();
    tr::UiObject* acceptCurrObjectOrigChanged();
    tr::UiObject* acceptCurrObjectEmptyTransl();
    /// accept curr object, all bugs gagged
    tr::UiObject* acceptCurrObjectAll();
    void revertCurrObject();
    void markAttentionCurrObject();
    void removeAttentionCurrObject();
    void runDecoder();
    void runTrash();
    // Menu: Go
    void goBack();
    void goNext();
    void goUp();
    void goFind();
    void goSearchAgain();
    void goAllWarnings();
    void goChangedUntransl();
    void goChangedOnly();
    void goUntranslOnly();
    void goAttention();
    void goChangedToday();
    void goMismatchNumber();
    void goCommentedByAuthor();
    void goCommentedByTranslator();
    // Menu: Original
    void addHostedFile();
    void addHostedGroup();
    void addSyncGroup();
    void addText();
    void doDelete();
    void doClone();
    void doLoadText();
    void clearGroup();
    void badShortcut();
    // Menu: Tools
    void translateWithOriginal();
    void extractOriginal();
    void switchOriginalAndTranslation();
    void resetKnownOriginals();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Ui::FmMain *ui;

    // Discrepancy images, completely programmatic now
    struct ImgBug {
        DblClickSvgWidget* ok = nullptr;
        DblClickSvgWidget* origChanged = nullptr;
        DblClickSvgWidget* emptyTransl = nullptr;
        DblClickSvgWidget* attention = nullptr;
        DblClickSvgWidget* mojibake = nullptr;
        DblClickSvgWidget* emptyOrig = nullptr;
        DblClickSvgWidget* invisible = nullptr;
        DblClickSvgWidget* multiline = nullptr;
        struct Space {
            DblClickSvgWidget* addBeg = nullptr;
            DblClickSvgWidget* delBeg = nullptr;
            DblClickSvgWidget* addEnd = nullptr;
            DblClickSvgWidget* delEnd = nullptr;
        } space;
    } imgBug;

    PrjTreeModel treeModel;
    std::shared_ptr<tr::Project> project;
    struct Search {
        std::unique_ptr<tr::FindCriterion> criterion;
        std::unique_ptr<ts::Result> result;
    } search;
    tr::UpdateInfo updateInfo;    
    std::unique_ptr<QTimer> timerBug;
    std::vector<QAction*> searchActions;

    Uptr<FmNew> fmNew;
    Uptr<FmDisambigPair> fmDisambigPair;
    Uptr<FmDecoder> fmDecoder;
    Uptr<FmFileFormat> fmFileFormat;
    Uptr<FmFind> fmFind;
    Uptr<FmProjectSettings> fmProjectSettings;
    Uptr<FmExtractOriginal> fmExtractOriginal;
    Uptr<FmSwitchOriginalAndTranslation> fmSwitchOriginalAndTranslation;
    Uptr<FmTranslateWithOriginal> fmTranslateWithOriginal;
    Uptr<FmMessage> fmMessage;
    Uptr<FmTrash> fmTrash;

    struct loadSetsCache {
        const void* fileKey = nullptr;
        CloningUptr<tf::FileFormat> format;
        tf::LoadTextsSettings text;
        tf::SyncInfo syncInfo;
    } loadSetsCache;
    tr::BugCache bugCache;
    std::atomic<bool> isChangingProgrammatically = false;
    QShortcut *shAddGroup = nullptr, *shAddText = nullptr,
              *shMarkAttention = nullptr;

    /// Adapts window’s layout to project type:
    /// original / full translation / (someday) patch translation
    void adaptLayout();

    /// Loads an UI object
    void loadObject(tr::UiObject& obj);
    void loadContext(tr::UiObject* lastSon);
    /// Writes UI to BugCache
    void uiToCache(tr::BugCache& r);
    /// Saves an UI object to project
    void acceptObject(tr::UiObject& obj, Flags<tr::Bug> bugsToRemove);
    /// Enables-disables UI actions according to current things edited
    void reenable();
    void reenableOnSelect();
    void reenableOnSelect(tr::UiObject* obj);
    void reenableOnSelect(tr::UiObject& obj) { reenableOnSelect(&obj); }
    void updateCaption();
    /// Returns parent group for addition, probably calling dialog form
    std::optional<std::shared_ptr<tr::VirtualGroup>> disambigGroup(std::u8string_view title);
    QModelIndex treeIndex();
    enum class EditMode : unsigned char { GROUP, TEXT };
    ///  Does what it needs to edit ORIGINAL (not translation)
    void startEditingOrig(const QModelIndex& index, EditMode editMode);
    void selectSmth();
    /// We made a project (new/load); plants it into UI
    void plantNewProject(std::shared_ptr<tr::Project>&& x);
    /// Enables-disables node editors en masse
    void setEditorsEnabled(bool x);
    /// Sets memo programmatically, w/o triggering modify    
    void setMemo(QWidget* wi, QPlainTextEdit* memo,
                 std::u8string_view placeholder,
                 std::u32string_view y);
    /// Same but for disabling components
    void banMemo(QWidget* wi, QPlainTextEdit* memo);
    void openFile(std::filesystem::path fname);
    void openFileFromHistory(unsigned i);    
    void openFileThrow(std::filesystem::path fname, OpenPlace& rPlace);
    void handleReferenceError(const char* text);
    void doBuild();
    bool checkSave(std::string_view caption);
    void plantSearchResult(
            std::unique_ptr<tr::FindCriterion> criterion,
            std::unique_ptr<ts::Result> x);
    void reflectUpdateInfo();
    void showBugs(Flags<tr::Bug> x);
    [[nodiscard]] PrjTreeModel::LockAll lockAll(RememberCurrent rem);
    tr::UiObject* acceptCurrObject(Flags<tr::Bug> bugsToRemove);
    void findBy(std::unique_ptr<tr::FindCriterion> crit);
    void setSearchAction(QAction* action, void (FmMain::* func)());
    void retrieveVersion();
    void updateSyncGroups();
    void updateOriginal();
    void updateReference();
    void clearReference();
    [[nodiscard]] QShortcut* addBadShortcut(QAction* action);
    void loadBugImages();
    [[nodiscard]] DblClickSvgWidget* loadBugWidget(
            const char* path, const char* description);
    void showMessageOverTree(std::u8string_view msg);
    void markAttentionCurrObjectEx(FuncBoolBool func);
};
