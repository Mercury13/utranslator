#pragma once

#include <QDialog>
#include "TrProject.h"

namespace Ui {
class FmDisambigPair;
}

class FmDisambigPair : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmDisambigPair;
public:
    explicit FmDisambigPair(QWidget *parent = nullptr);
    ~FmDisambigPair() override;

    std::optional<std::shared_ptr<tr::VirtualGroup>> exec(
            std::u8string_view title,
            const tr::Pair<tr::VirtualGroup>& groups);

private:
    Ui::FmDisambigPair *ui;

    using Super::exec;
};
