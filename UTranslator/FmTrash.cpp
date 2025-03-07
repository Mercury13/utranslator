#include "FmTrash.h"
#include "ui_FmTrash.h"

// Qt GUI
#include <QDialogButtonBox>
#include <QPushButton>

// Qt ex
#include "QtConsts.h"

// Project
#include "TrProject.h"

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


bool FmTrash::isSameTrash(const tr::Trash& x) noexcept
{
    bool r = (x.passport.get() == oldTrash.passport
           && x.size() >= oldTrash.size);
    oldTrash.passport = x.passport.get();
    oldTrash.size = x.size();
    return r;
}


void FmTrash::exec(tr::Trash& trash, QstrObject* obj, TrashChannel channel)
{
    // Enable-disable OK
    auto btOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    btOk->setEnabled(obj && trash.hasSmth());

    /// @todo [urgent] exec trash
    Super::exec();

    if (obj) {
        switch (channel) {
        case TrashChannel::ORIGINAL:
            /// @todo [urgent] original
            break;
        case TrashChannel::TRANSLATION:
            /// @todo [urgent] translation
            break;
        }
    }
}
