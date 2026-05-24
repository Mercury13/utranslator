#ifndef FMTRASH_H
#define FMTRASH_H

// Qt
#include <QDialog>
#include <QAbstractTableModel>

// C++
#include <concepts>

namespace Ui {
class FmTrash;
}

class StrObject;

namespace tr {
    struct Trash;
    struct TrashLine;
    struct Passport;
}

enum class TrashChannel : unsigned char {
    ORIGINAL, TRANSLATION,
    NOMATTER = ORIGINAL };

struct TrashModel final : public QAbstractTableModel
{
public:
    // QAbstractTableModel
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setTrash(tr::Trash& x);
    void removeTrash();
    size_t knownSize() const { return oldTrash.size; }
    bool isRowGood(int) const;
    bool isRowGood(unsigned) const = delete;
    bool isRowBad(int x) const { return !isRowGood(x); }
    bool isRowBad(unsigned) const = delete;

    // Actually model-local
    template <std::integral T>
    const tr::TrashLine* at (T rw) const;
private:
    const tr::Trash* trash = nullptr;

    struct {
        std::shared_ptr<tr::Passport> passport {};
        size_t size = 0;
    } oldTrash;

    struct TrashDiff {
        /// As we add new items to the end only, diff is enough
        size_t oldSize = 0, newSize = 0;
        bool isSame = false;
    };
    TrashDiff trashDiff(const tr::Trash& x) noexcept;
    void changeTrash(const tr::Trash& x);
};

class FmTrash : public QDialog        
{
    Q_OBJECT
    using This = FmTrash;
    using Super = QDialog;
public:
    explicit FmTrash(QWidget *parent = nullptr);
    ~FmTrash() override;
    void exec(tr::Trash& trash, StrObject* obj, TrashChannel channel);
private:
    Ui::FmTrash *ui;
    TrashModel model;
    bool canAccept = false;

    using Super::exec;
private slots:
    void treeDblClicked(const QModelIndex& index);
    void treeCurrentChanged(const QModelIndex& newSel);
};

#endif // FMTRASH_H
