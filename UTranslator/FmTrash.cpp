#include "FmTrash.h"
#include "ui_FmTrash.h"

#include "QtConsts.h"

FmTrash::FmTrash(QWidget *parent) :
    Super(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmTrash)
{
    ui->setupUi(this);
}

FmTrash::~FmTrash()
{
    delete ui;
}

void FmTrash::exec(TrashChannel channel)
{
    /// @todo [urgent] exec trash
    Super::exec();
}
