#pragma once

// Qt
#include <QTreeView>
#include <QStyledItemDelegate>

// Translation
#include "TrProject.h"
#include "TrWrappers.h"

template <class T>
struct Thing {
    std::shared_ptr<T> subj;
    QModelIndex index;

    Thing() = default;
    Thing(std::shared_ptr<T> aSubj, const QModelIndex& aIndex)
        : subj(std::move(aSubj)), index(aIndex) {}
    explicit operator bool() const { return static_cast<bool>(subj); }
};

enum class RememberCurrent { NO, YES };

enum class PrjColClass {
    ID, ORIGINAL, REFERENCE, TRANSLATION
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
    Thing<tr::Group> addGroup(const std::shared_ptr<tr::VirtualGroup>& parent);
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
    const std::shared_ptr<tr::Project>& project() const { return prj; }

    class LockAll
    {
    public:
        LockAll(const LockAll&) = delete;
        /// Move is better, but C++17 return value optimization works here, so OK
        LockAll(LockAll&& x) noexcept = delete;
        LockAll& operator = (const LockAll&) = delete;
        LockAll& operator = (LockAll&& x) noexcept = delete;

        ~LockAll();
    private:
        friend class PrjTreeModel;
        LockAll(PrjTreeModel& x, QTreeView* aView, RememberCurrent aRem);
        PrjTreeModel* owner = nullptr;  ///< why ptr? Maybe we’ll implement move-assignment
        QTreeView* view = nullptr;
        RememberCurrent rem;
    };

    [[nodiscard]] LockAll lock(QTreeView* view, RememberCurrent rem)
        { return LockAll(*this, view, rem); }

private:
    static constexpr int DUMMY_COL = 0; ///< for QAbstractItemModel.parent
    std::shared_ptr<tr::Project> prj;   ///< will hold old project
    std::vector<PrjColClass> colMeanings;
    mutable tw::Flyweight fly;
};


#define FILE_PREFIX u8"file"
#define FILE_SUFFIX u8".txt"
#define FILE_INITIAL   FILE_PREFIX "0" FILE_SUFFIX
