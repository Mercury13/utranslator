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

// Project-local
#include "TrProject.h"


namespace Ui {
class FmExtractOriginal;
}

enum class TextChannel { ORIG, TRANSL, TRANSL_ORIG };

enum class CommentChannel { AUTHOR, AUTHOR_TRANSL, TRANSL, TRANSL_AUTHOR };

struct ExtractOriginalSets {
    TextChannel textChannel;
    CommentChannel commentChannel;
};

class FmExtractOriginal : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmExtractOriginal;
public:
    explicit FmExtractOriginal(QWidget *parent = nullptr);
    ~FmExtractOriginal();
    std::optional<ExtractOriginalSets> exec(int dummy = 0);

private:
    Ui::FmExtractOriginal *ui;
    EcRadio<TextChannel> radioText;
    EcRadio<CommentChannel> radioComment;

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

extern const ec::Array<const ProcessTr*, TextChannel> extractOriginalTr;
extern const ec::Array<const ProcessCm*, CommentChannel> extractOriginalCm;

#endif // FMEXTRACTORIGINAL_H
