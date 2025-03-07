#ifndef FMTRASH_H
#define FMTRASH_H

#include <QDialog>

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

class FmTrash : public QDialog        
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit FmTrash(QWidget *parent = nullptr);
    ~FmTrash() override;
    void exec(tr::Trash& trash, QstrObject* obj, TrashChannel channel);
private:
    Ui::FmTrash *ui;
    using Super::exec;

    struct {
        tr::Passport* passport = nullptr;
        size_t size = 0;
    } oldTrash;

    bool isSameTrash(const tr::Trash& x) noexcept;
};

#endif // FMTRASH_H
