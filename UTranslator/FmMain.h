#pragma once

// Qt
#include <QMainWindow>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

// Project
// No one uses FmMain → you can import EVERYTHING
#include "TrProject.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class FmNew;
class FmDisambigPair;

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

class FmMain :
        public QMainWindow,
        public ModListener
{
    Q_OBJECT
    using Super = QMainWindow;
    using This = FmMain;
public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

    void modStateChanged(ModState oldState, ModState newState) override;
private slots:
    // Tree etc.
    void treeCurrentChanged(
            const QModelIndex& current, const QModelIndex& former);

    // Menu: File
    void doNew();
    void doOpen();
    void doSave();
    void doSaveAs();
    // Menu: Edit
    void acceptCurrObject();
    void revertCurrObject();
    // Menu: Go
    void goBack();
    void goNext();
    // Menu: Original
    void addHostedFile();
    void addHostedGroup();
    void addText();
    void doDelete();
private:
    Ui::FmMain *ui;

    PrjTreeModel treeModel;
    std::shared_ptr<tr::Project> project;

    Uptr<FmNew> fmNew;
    Uptr<FmDisambigPair> fmDisambigPair;

    /// Adapts window’s layout to project type:
    /// original / full translation / (someday) patch translation
    void adaptLayout();

    /// Loads an UI object
    void loadObject(tr::UiObject& obj);
    /// Saves an UI object to project
    void acceptObject(tr::UiObject& obj);
    /// Enables-disables UI actions according to current things edited
    void reenable();
    void updateCaption();
    /// Returns parent group for addition, probably calling dialog form
    std::optional<std::shared_ptr<tr::VirtualGroup>> disambigGroup(std::u8string_view title);
    QModelIndex treeIndex();
    ///  Does what it needs to edit ORIGINAL (not translation)
    void startEditingOrig(const QModelIndex& index);
    void selectSmth();
    void plantNewProject(std::shared_ptr<tr::Project>&& x);
};
