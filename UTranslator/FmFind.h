#pragma once

#include <QDialog>

#include "TrDefines.h"

namespace Ui {
class FmFind;
}

struct FindOptions {
    QString text;
    struct Channels {
        bool id = false, original = false, importersAuthorsComment = false,
             translation = false, translatorsComment = false;
        static const Channels NONE;
        // default C++20 op==
        bool operator == (const Channels&) const = default;
    } channels;
    struct Options {
        bool matchCase = false;
    } options;

    explicit operator bool () const { return !text.isEmpty(); }
    bool areSet() const { return !text.isEmpty() && channels != Channels::NONE; }
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
private slots:
    void acceptIf();
private:
    Ui::FmFind *ui;
    FindOptions opts;
    bool isTransl = false;

    using Super::exec;
    void copyTo(FindOptions& r);
};
