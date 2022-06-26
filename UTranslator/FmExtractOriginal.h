#ifndef FMEXTRACTORIGINAL_H
#define FMEXTRACTORIGINAL_H

// STL
#include <optional>

// Qt
#include <QDialog>

// Qt ex
#include "QtMultiRadio.h"

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

#endif // FMEXTRACTORIGINAL_H
