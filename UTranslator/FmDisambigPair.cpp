#include "FmDisambigPair.h"
#include "ui_FmDisambigPair.h"

#include "u_Qstrings.h"
#include "QtConsts.h"

FmDisambigPair::FmDisambigPair(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmDisambigPair)
{
    ui->setupUi(this);
    connect(ui->radio1, &DblClickRadio::doubleClicked, this, &This::accept);
    connect(ui->radio2, &DblClickRadio::doubleClicked, this, &This::accept);
}

FmDisambigPair::~FmDisambigPair()
{
    delete ui;
}

std::optional<std::shared_ptr<tr::VirtualGroup>> FmDisambigPair::exec(
        std::u8string_view title,
        const tr::Pair<tr::VirtualGroup>& groups)
{
    if (!groups.first || !groups.second)
        throw std::logic_error("[FmDisambigPair.exec] First and second should both exist");

    setWindowTitle(str::toQ(title));
    if (groups.first->id == groups.second->id) {    // Names are identical, what to do?
        ui->lbAdding->setText(u8"Add…");
        ui->radio1->setText("Next to current group");
        ui->radio2->setText("Inside current group");
    } else {    // Names differ, as usual
        ui->lbAdding->setText(u8"Add in…");
        ui->radio1->setText(str::toQ(groups.first->id));
        ui->radio2->setText(str::toQ(groups.second->id));
    }
    ui->radio1->setChecked(true);
    ui->radio1->setFocus();

    if (Super::exec()) {
        if (ui->radio1->isChecked()) {
            return groups.first;
        } else {
            return groups.second;
        }
    } else {
        return std::nullopt;
    }
}
