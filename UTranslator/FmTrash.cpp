#include "FmTrash.h"
#include "ui_FmTrash.h"

// Qt GUI
#include <QDialogButtonBox>
#include <QPushButton>

// Qt ex
#include "QtConsts.h"

// Project
#include "TrVirtuals.h"

// Utils
#include "u_Qstrings.h"
#include "Tools/QstrObject.h"

///  Simple conversion signed → unsigned
template <std::integral T>
auto toUnsigned(T x) -> std::make_unsigned_t<T>
    { return x; }

///  Saturating conversion signed → unsigned (neg→0)
template <std::integral T>
auto satUnsigned(T x) -> std::make_unsigned_t<T>
{
    static_assert(std::is_signed_v<T>);
    return (x < 0) ? 0 : x;
}


///// TrashModel ///////////////////////////////////////////////////////////////


int TrashModel::rowCount(const QModelIndex&) const
{
    if (!trash)
        return 0;
    return trash->size();
}


enum {
    COL_ID, COL_ORIG, COL_TRANSL,
    COL_N
};


int TrashModel::columnCount(const QModelIndex&) const
{
    return COL_N;
}


QVariant TrashModel::data(const QModelIndex &index, int role) const
{
    if (!trash || toUnsigned(index.row()) >= satUnsigned(rowCount({})))
        return {};
    switch (role) {
    case Qt::DisplayRole: {
            const auto& line = trash->data[index.row()];
            switch (index.column()) {
            case COL_ID:
                if (line.idChain.empty())
                    return {};
                return str::toQ(line.idChain.back());
            case COL_ORIG:
                return str::toQ(line.tr.original);
            case COL_TRANSL:
                return str::toQ(line.tr.translationSv());
            default:
                return {};
            }
        }
    default:
        return {};
    }
}


QVariant TrashModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal
            || role != Qt::DisplayRole) {
        return {};
    }
    static constinit const char* names[] {
        "Id", "Original", "Translation",
    };
    static_assert(std::size(names) == COL_N);
    if (auto sec = toUnsigned(section); sec < COL_N) {
        return names[sec];
    }
    return {};
}


void TrashModel::setTrash(tr::Trash& x)
{
    beginResetModel();
    trash = &x;
    endResetModel();
}

void TrashModel::removeTrash()
{
    beginResetModel();
    trash = nullptr;
    endResetModel();
}


///// FmTrash //////////////////////////////////////////////////////////////////


FmTrash::FmTrash(QWidget *parent) :
    Super(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmTrash)
{
    ui->setupUi(this);

    ui->treeAll->setModel(&model);

    connect(ui->treeAll, &QTreeView::doubleClicked, this, &This::treeDblClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
}

FmTrash::~FmTrash()
{
    delete ui;
}


void FmTrash::treeDblClicked(const QModelIndex& index)
{
    if (canAccept && index.isValid()) {
        accept();
    }
}


bool FmTrash::isSameTrash(const tr::Trash& x) noexcept
{
    bool r = (x.passport.get() == oldTrash.passport
           && x.size() >= oldTrash.size);
    oldTrash.passport = x.passport.get();
    oldTrash.size = x.size();
    return r;
}


void FmTrash::exec(tr::Trash& trash, StrObject* obj, TrashChannel channel)
{
    // Enable-disable OK
    auto btOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    canAccept = obj && trash.hasSmth();
    btOk->setEnabled(canAccept);

    model.setTrash(trash);
    if (trash.hasSmth()) {
        ui->treeAll->setCurrentIndex(model.index(0, 0));
        ui->treeAll->setFocus();
    }

    auto result = Super::exec();

    if (obj && result) {
        auto index = toUnsigned(ui->treeAll->currentIndex().row());
        if (index < trash.size()) {
            auto& line = trash.data[index];
            switch (channel) {
            case TrashChannel::ORIGINAL:
                obj->set(line.tr.original);
                break;
            case TrashChannel::TRANSLATION:
                obj->set(line.tr.translationSv());
                break;
            }
        }
    }

    model.removeTrash();
}
