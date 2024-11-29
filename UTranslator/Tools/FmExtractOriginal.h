#ifndef FMEXTRACTORIGINAL_H
#define FMEXTRACTORIGINAL_H

// STL
#include <optional>

// Qt
#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

// Translation
#include "TrUtils.h"


namespace Ui {
class FmExtractOriginal;
}

class FmExtractOriginal : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmExtractOriginal;
public:
    explicit FmExtractOriginal(QWidget *parent = nullptr);
    ~FmExtractOriginal();
    std::optional<tr::eo::Sets> exec(int dummy = 0);

private:
    Ui::FmExtractOriginal *ui;
    EcRadio<tr::eo::Text> radioText;
    EcRadio<tr::eo::Comment> radioComment;

    using Super::exec;
};


#endif // FMEXTRACTORIGINAL_H
