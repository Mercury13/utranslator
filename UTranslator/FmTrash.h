#ifndef FMTRASH_H
#define FMTRASH_H

// Qt
#include <QDialog>
#include <QAbstractTableModel>

namespace Ui {
class FmTrash;
}

class QstrObject;

namespace tr {
    struct Trash;
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
private:
    tr::Trash* trash = nullptr;
};

class FmTrash : public QDialog        
{
    Q_OBJECT
    using This = FmTrash;
    using Super = QDialog;
public:
    explicit FmTrash(QWidget *parent = nullptr);
    ~FmTrash() override;
    void exec(tr::Trash& trash, QstrObject* obj, TrashChannel channel);
private:
    Ui::FmTrash *ui;
    using Super::exec;

    TrashModel model;

    struct {
        tr::Passport* passport = nullptr;
        size_t size = 0;
    } oldTrash;

    bool isSameTrash(const tr::Trash& x) noexcept;
};

#endif // FMTRASH_H
