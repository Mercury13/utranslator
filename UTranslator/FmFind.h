#pragma once

#include <QDialog>

#include "TrDefines.h"

namespace Ui {
class FmFind;
}

struct FindOptions {
    QString text;
    struct Channels {
        bool id = false, original = false, authorsComment = false,
             translation = false, translatorsComment = false;
    } channels;
    struct Options {
        bool matchCase = false;
    } options;

    explicit operator bool () const { return !text.isEmpty(); }
};

class FmFind : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmFind;
public:
    explicit FmFind(QWidget *parent = nullptr);
    ~FmFind() override;

    FindOptions exec(tr::PrjType prjType);

private:
    Ui::FmFind *ui;

    using Super::exec;
};
