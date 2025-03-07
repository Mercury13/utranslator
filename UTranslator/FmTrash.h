#ifndef FMTRASH_H
#define FMTRASH_H

#include <QDialog>

namespace Ui {
class FmTrash;
}

class QstrObject;

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
    void exec(TrashChannel channel);
private:
    Ui::FmTrash *ui;
    using Super::exec;
};

#endif // FMTRASH_H
