#ifndef FMEXTRACTORIGINAL_H
#define FMEXTRACTORIGINAL_H

// STL
#include <optional>

// Qt
#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

// Libs
#include "u_EcArray.h"

// Translation
#include "TrUtils.h"
#include "TrProject.h"


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


class ProcessTr {   // interface
public:
    virtual void act(tr::Translatable& x) const = 0;
    virtual ~ProcessTr() = default;
};

class ProcessCm {   // interface
public:
    virtual void act(tr::Comments& x) const = 0;
    virtual ~ProcessCm() = default;
};

extern const ec::Array<const ProcessTr*, tr::eo::Text> extractOriginalTr;
extern const ec::Array<const ProcessCm*, tr::eo::Comment> extractOriginalCm;

#endif // FMEXTRACTORIGINAL_H