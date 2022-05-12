#pragma once

// Qt
#include <QMainWindow>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QUrl>

// Libs
#include "u_Uptr.h"

// Project
// No one uses FmMain → you can import EVERYTHING
#include "TrProject.h"
#include "TrWrappers.h"

// Project-local
#include "History.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class FmNew;
class FmDisambigPair;
class FmDecoder;
class FmFileFormat;
class FmFind;
class FmProjectSettings;
class QPlainTextEdit;

namespace ts {
    class Result;
}

template <class T>
struct Thing {
    std::shared_ptr<T> subj;
    QModelIndex index;

    Thing() = default;
    Thing(std::shared_ptr<T> aSubj, const QModelIndex& aIndex)
        : subj(std::move(aSubj)), index(aIndex) {}
    explicit operator bool() const { return static_cast<bool>(subj); }
};

class PrjTreeModel final : public QAbstractItemModel, public QStyledItemDelegate
{
public:
    PrjTreeModel();

    /// Converts index to object
    /// @param [in] index   QAbstractItemModel’s index
    /// @param [in] dflt    What’s instead of root
    tr::UiObject* toObjOr(const QModelIndex& index, tr::UiObject* dflt) const;

    /// Equiv. to toObjOr(index, project.get())
    /// Never nullptr.
    tr::UiObject* toObj(const QModelIndex& index) const;
    QModelIndex toIndex(const tr::UiObject& obj, int col) const;
    QModelIndex toIndex(const tr::UiObject* p, int col) const;
    QModelIndex toIndex(const std::shared_ptr<tr::UiObject>& p, int col) const
            { return toIndex(p.get(), col); }

    // QAbstractItemModel
    QModelIndex index(int row, int column,
                const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    // QAbstractItemDelegate
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    void setProject(std::shared_ptr<tr::Project> aProject);
    /// @return  s_p to new file
    Thing<tr::File> addHostedFile();
    /// @return  s_p to new group
    Thing<tr::Group> addHostedGroup(const std::shared_ptr<tr::VirtualGroup>& parent);
    /// @return  s_p to new text
    Thing<tr::Text> addText(const std::shared_ptr<tr::VirtualGroup>& parent);
    /// @return  [+] s_p to extracted object  [0] nothing happened
    std::shared_ptr<tr::Entity> extract(tr::UiObject* obj);
    struct CloneResult {
        tr::CloneErr err;       /// OK, or error reason
        QModelIndex index;      /// OK: new index    BAD: do not use
        tr::ObjType objType;    /// OK: object type  BAD: do not use
    };
    /// Clones this object
    CloneResult doClone(const QModelIndex& index);
    struct MoveResult {
        QModelIndex that {};
        QModelIndex next {};
        explicit operator bool() const { return that.isValid(); }
    };
    MoveResult moveUp(const QModelIndex& index);
    MoveResult moveDown(const QModelIndex& index);
    QModelIndex clearGroup(tr::UiObject* obj);

    struct LockAll {
        LockAll(const LockAll&) = delete;
        /// Move is better, but C++17 RVO works here, so OK
        LockAll(LockAll&& x) noexcept = delete;
        LockAll& operator = (const LockAll&) = delete;
        LockAll& operator = (LockAll&& x) noexcept = delete;

        ~LockAll() {
            if (owner) {
                owner->endResetModel();
            }
        }
    private:
        friend class PrjTreeModel;
        LockAll(PrjTreeModel& x) : owner(&x)
            { owner->beginResetModel();  }
        PrjTreeModel* owner = nullptr;
    };

    LockAll lock() { return LockAll(*this); }

private:
    static constexpr int DUMMY_COL = 0;
    std::shared_ptr<tr::Project> project;   // will hold old project
    mutable tw::Flyweight fly;
};

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
    // Menu: Edit
    tr::UiObject* acceptCurrObject();
    void revertCurrObject();
    // Menu: Go
    void goBack();
    void goNext();
    void goUp();
    void goFind();
    // Menu: Original
    void addHostedFile();
    void addHostedGroup();
    void addText();
    void doDelete();
    void doClone();
    void doLoadText();
    void clearGroup();
    // Menu: Tools
    void runDecoder();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Ui::FmMain *ui;

    PrjTreeModel treeModel;
    std::shared_ptr<tr::Project> project;
    std::unique_ptr<ts::Result> searchResult;
    tr::UpdateInfo updateInfo;

    Uptr<FmNew> fmNew;
    Uptr<FmDisambigPair> fmDisambigPair;
    Uptr<FmDecoder> fmDecoder;
    Uptr<FmFileFormat> fmFileFormat;
    Uptr<FmFind> fmFind;
    Uptr<FmProjectSettings> fmProjectSettings;

    struct loadSetsCache {
        void* fileKey = nullptr;
        CloningUptr<tf::FileFormat> format;
        tf::LoadTextsSettings text;
    } loadSetsCache;
    std::atomic<bool> isChangingProgrammatically = false;

    /// Adapts window’s layout to project type:
    /// original / full translation / (someday) patch translation
    void adaptLayout();

    /// Loads an UI object
    void loadObject(tr::UiObject& obj);
    void loadContext(tr::UiObject* lastSon);
    /// Saves an UI object to project
    void acceptObject(tr::UiObject& obj);
    /// Enables-disables UI actions according to current things edited
    void reenable();
    void updateCaption();
    /// Returns parent group for addition, probably calling dialog form
    std::optional<std::shared_ptr<tr::VirtualGroup>> disambigGroup(std::u8string_view title);
    QModelIndex treeIndex();
    enum class EditMode { GROUP, TEXT };
    ///  Does what it needs to edit ORIGINAL (not translation)
    void startEditingOrig(const QModelIndex& index, EditMode editMode);
    void selectSmth();
    /// We made a project (new/load); plants it into UI
    void plantNewProject(std::shared_ptr<tr::Project>&& x);
    /// Enables-disables node editors en masse
    void setEditorsEnabled(bool x);
    /// Sets memo programmatically, w/o triggering modify
    void setMemo(QWidget* wi, QPlainTextEdit* memo, std::u8string_view y);
    /// Same but for disabling components
    void banMemo(QWidget* wi, QPlainTextEdit* memo);
    void openFile(std::filesystem::path fname);
    void doBuild();
    bool checkSave(std::string_view caption);
    void plantSearchResult(
            const QString& caption, std::unique_ptr<ts::Result> x);
    void reflectUpdateInfo();
    void showBugs(Flags<tr::Bug> x);
};
