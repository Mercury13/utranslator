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


///// TrashModel ///////////////////////////////////////////////////////////////


template <std::integral T>
const tr::TrashLine* TrashModel::at (T rw) const {
    if (trash && isRowGood(rw)) {
        return &trash->data[rw];
    } else {
        return nullptr;
    }
}


int TrashModel::rowCount(const QModelIndex&) const
{
    return knownSize();
}


enum {
    COL_ID, COL_ORIG, COL_TRANSL,
    COL_N
};


int TrashModel::columnCount(const QModelIndex&) const
{
    return COL_N;
}


bool TrashModel::isRowGood(int x) const
{
    auto rc = rowCount({});
    return (rc > 0 && x < rc);
}


QVariant TrashModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
            const auto line = at(index.row());
            if (!line)
                return {};
            switch (index.column()) {
            case COL_ID:
                if (line->chain.ids.empty())
                    return {};
                return str::toQ(line->chain.ids.back());
            case COL_ORIG:
                return str::toQ(line->tr.original);
            case COL_TRANSL:
                return str::toQ(line->tr.translationSv());
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
    auto diff = trashDiff(x);
    if (diff.isSame) {
        if (diff.oldSize == diff.newSize) {
            changeTrash(x);
            emit dataChanged({}, {});
        } else {
            beginInsertRows({}, diff.oldSize, diff.newSize - 1);
            changeTrash(x);
            endInsertRows();
        }
    } else {
        beginResetModel();
        changeTrash(x);
        endResetModel();
    }
}


void TrashModel::removeTrash()
{
    trash = nullptr;
    emit dataChanged({}, {});
}


TrashModel::TrashDiff TrashModel::trashDiff(const tr::Trash& x) noexcept
{
    return {
        .oldSize = oldTrash.size,
        .newSize = x.size(),
        .isSame = (x.passport == oldTrash.passport
                   && x.size() >= oldTrash.size),
    };
}


void TrashModel::changeTrash(const tr::Trash& x)
{
    oldTrash.passport = x.passport;
    oldTrash.size = x.size();
    trash = &x;
}


///// FmTrash //////////////////////////////////////////////////////////////////


FmTrash::FmTrash(QWidget *parent) :
    Super(parent, QDlgType::SIZEABLE),
    ui(new Ui::FmTrash)
{
    ui->setupUi(this);

    ui->treeAll->setModel(&model);

    connect(ui->treeAll, &QTreeView::doubleClicked, this, &This::treeDblClicked);
    connect(ui->treeAll->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &This::treeCurrentChanged);
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


void FmTrash::exec(tr::Trash& trash, StrObject* obj, TrashChannel channel)
{
    // Enable-disable OK
    auto btOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    canAccept = obj && trash.hasSmth();
    btOk->setEnabled(canAccept);

    model.setTrash(trash);
    if (trash.hasSmth() && !ui->treeAll->currentIndex().isValid()) {
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


namespace {

    QString chainToString(const tr::StoringIdChain& x)
    {
        auto r = str::toQ(x.fileName);
        if (!x.ids.empty()) {
            if (!r.isEmpty()) {
                // Both non-empty file and ID chain
                r += ": ";
            }
            for (auto& v : x.ids) {
                if (&v != &x.ids.front())
                    r += " → ";
                if (v.empty()) {
                    r += "⌀";
                } else {
                    r += str::toQ(v);
                }
            }
        }
        return r;
    }

}   // anon namespace


void FmTrash::treeCurrentChanged(const QModelIndex& newSel)
{
    if (auto line = model.at(newSel.row())) {
        // Have line
        ui->edIdChain->setText(chainToString(line->chain));
    } else {
        // No line
        ui->edIdChain->clear();
    }
}
