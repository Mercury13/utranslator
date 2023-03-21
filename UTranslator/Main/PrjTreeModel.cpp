// My header
#include "PrjTreeModel.h"

// Qt ex
#include "QModels.h"

// Libs
#include "u_Qstrings.h"
#include "u_EcArray.h"

// Data
#include "d_Strings.h"

///// LockAll //////////////////////////////////////////////////////////////////

namespace {

    void saveViewStateRec(
            PrjTreeModel& model,
            QTreeView* view,
            tr::UiObject* obj)
    {
        if (!obj)
            throw std::logic_error("[recurseViewState] obj=null!");
        auto index = model.toIndex(obj, 0);

        bool areAllCollapsed = true;
        size_t nc = obj->nChildren();
        for (size_t i = 0; i < nc; ++i) {
            auto child = obj->child(i);
            saveViewStateRec(model, view, child.get());
            switch (child->cache.treeUi.expandState) {
            case tr::ExpandState::COLLAPSED:
            case tr::ExpandState::ALL_COLLAPSED:
            case tr::ExpandState::UNKNOWN:
                break;
            case tr::ExpandState::EXPANDED:
                areAllCollapsed = false;
            }
        }

        if (index.isValid()) {
            if (nc == 0) {
                obj->cache.treeUi.expandState = tr::ExpandState::UNKNOWN;
            } if (view->isExpanded(index)) {
                obj->cache.treeUi.expandState = tr::ExpandState::EXPANDED;
            } else {
                obj->cache.treeUi.expandState = (areAllCollapsed)
                        ? tr::ExpandState::ALL_COLLAPSED
                        : tr::ExpandState::COLLAPSED;
            }
        } else {
            obj->cache.treeUi.expandState = tr::ExpandState::COLLAPSED;  // will do nothing when restoring
        }
    }

    void saveViewState(
            PrjTreeModel& model,
            QTreeView* view,
            RememberCurrent rem)
    {
        if (auto prj = model.project()) {
            saveViewStateRec(model, view, prj.get());
            if (rem != RememberCurrent::NO) {
                auto index = view->currentIndex();
                if (index.isValid()) {
                    auto obj = model.toObj(index)->selfUi();
                    obj->cache.treeUi.currObject.reset();
                    while (true) {
                        auto parent = obj->parent();
                        if (!parent)
                            break;
                        parent->cache.treeUi.currObject = obj;
                        obj = parent;
                    }
                } else {
                    prj->cache.treeUi.currObject.reset();
                }
            }
        }
    }

    void restoreViewStateRec(
            PrjTreeModel& model,
            QTreeView* view,
            tr::UiObject* obj)
    {
        if (!obj)
            throw std::logic_error("[recurseViewState] obj=null!");
        auto index = model.toIndex(obj, 0);

        switch (obj->cache.treeUi.expandState) {
        case tr::ExpandState::UNKNOWN:
        case tr::ExpandState::ALL_COLLAPSED:
            return;
        case tr::ExpandState::EXPANDED:
            view->expand(index);
            break;
        case tr::ExpandState::COLLAPSED:
            break;
        }

        size_t nc = obj->nChildren();
        for (size_t i = 0; i < nc; ++i) {
            auto child = obj->child(i);
            restoreViewStateRec(model, view, child.get());
        }
    }

    void restoreViewState(
            PrjTreeModel& model,
            QTreeView* view,
            RememberCurrent rem)
    {
        if (auto prj = model.project()) {
            restoreViewStateRec(model, view, prj.get());
            if (rem != RememberCurrent::NO) {
                std::shared_ptr<tr::UiObject> curr = prj;
                while (true) {
                    auto child = curr->cache.treeUi.currObject.lock();
                    if (!child || child->parent() != curr)
                        break;
                    curr = child;
                }
                if (curr == prj) {
                    qmod::selectFirstAmbiguous(view);
                } else {
                    view->setCurrentIndex(model.toIndex(curr, 0));
                }
            }
        }
    }

}   // anon namespace


PrjTreeModel::LockAll::LockAll(
        PrjTreeModel& x, QTreeView* aView, RememberCurrent aRem)
    : owner(&x), view(aView), rem(aRem)
{
    saveViewState(x, view, rem);
    owner->beginResetModel();
}

PrjTreeModel::LockAll::~LockAll()
{
    try {
        if (owner) {
            owner->buildColMeanings();
            owner->endResetModel();
            restoreViewState(*owner, view, rem);
        }
    } catch (...) {
        std::terminate();   // -warn
    }
}


///// PrjTreeModel /////////////////////////////////////////////////////////////

namespace {

    constinit const tw::L10n treeL10n {
        .untranslated = S8(STR_UNTRANSLATED),
        .emptyString = S8(STR_EMPTY_STRING),
    };

}   // anon namespace


PrjTreeModel::PrjTreeModel()
{
    fly.setL10n(treeL10n);
    buildColMeanings();
}


void PrjTreeModel::setProject(std::shared_ptr<tr::Project> aProject)
{
    beginResetModel();
    prj = std::move(aProject);
    buildColMeanings();
    endResetModel();
}

void PrjTreeModel::getColMeanings(SafeVector<PrjColClass>& r) const
{
    r.clear();
    r.push_back(PrjColClass::ID);
    r.push_back(PrjColClass::ORIGINAL);
    if (prj) {
        if (prj->info.hasReference())
            r.push_back(PrjColClass::REFERENCE);
        if (prj->info.isTranslation())
            r.push_back(PrjColClass::TRANSLATION);
    }
}

void PrjTreeModel::buildColMeanings()
{
    getColMeanings(colMeanings);
}

std::optional<TempProps> PrjTreeModel::checkProps() const
{
    TempProps r;
    getColMeanings(r.colMeanings);
    if (colMeanings == r.colMeanings)
        return std::nullopt;
    return r;
}

tr::UiObject* PrjTreeModel::toObjOr(
        const QModelIndex& index, tr::UiObject* dflt) const
{
    auto ptr = index.internalPointer();
    if (!ptr)
        return dflt;
    auto r = static_cast<tr::UiObject*>(ptr);
    r->checkCanary();
    if (r->objType() == tr::ObjType::PROJECT)
        return dflt;
    return r;
}

tr::UiObject* PrjTreeModel::toObj(const QModelIndex& index) const
{
    auto r = toObjOr(index, prj.get());
    if (!r)
        throw std::logic_error("[toObj] Somehow got nullptr");
    return r;
}


QModelIndex PrjTreeModel::toIndex(const tr::UiObject& obj, int col) const
{
    if (obj.objType() == tr::ObjType::PROJECT)
        return {};
    return createIndex(obj.cache.index, col, &obj);
}

QModelIndex PrjTreeModel::toIndex(const tr::UiObject* p, int col) const
{
    if (!p)
        return {};
    return toIndex(*p, col);
}

QModelIndex PrjTreeModel::index(int row, int col,
            const QModelIndex &parent) const
{
    auto obj = toObj(parent);
    auto child = obj->child(row);
    return toIndex(child, col);
}

QModelIndex PrjTreeModel::parent(const QModelIndex &child) const
{
    auto obj = toObj(child);
    return toIndex(obj->parent(), DUMMY_COL);
}

int PrjTreeModel::rowCount(const QModelIndex &parent) const
{
    auto obj = toObj(parent);
    return obj->nChildren();
}

int PrjTreeModel::columnCount(const QModelIndex &) const
{
    return colMeanings.size();
}

namespace {
    constexpr QColor BG_MODIFIED { 0xFA, 0xF0, 0xE6 };

    QString brushLineEnds(std::u8string_view x)
    {
        auto r = str::toQ(x);
        r.replace('\n', QChar{L'¶'});
        return r;
    }

    QString brushLineEnds(const tw::TranslObj& x)
    {
        auto r = str::toQ(x.str());
        if (x.mayContainEols()) {
            r.replace('\n', QChar{L'¶'});
        }
        return r;
    }

    constinit const QColor fgColors[] = {
        QColor(),               // NORMAL,
        { 0x77, 0x00, 0x00 },   // UNTRANSLATED_CAT — some dark red
        { 0xDC, 0x14, 0x3C },   // ATTENTION — HTML crimson
        { 0x00, 0x80, 0x00 },   // OK — dumb green
        { 0x69, 0x69, 0x69 },   // STATS — HTML dim gray
        { 0xCC, 0xCC, 0xCC },   // LIGHT — HTML light gray
    };
    static_assert(std::size(fgColors) == tw::Fg_N);
}

QVariant PrjTreeModel::data(const QModelIndex &index, int role) const
{
    if (!prj)    // No project
        return {};
    switch (role) {
    case Qt::DisplayRole: {
            auto obj = toObj(index);
            switch (colMeanings.safeGetV(index.column(), PrjColClass::DUMMY)) {
            case PrjColClass::DUMMY:
                return "????????";
            case PrjColClass::ID:
                return str::toQ(obj->idColumn());
            case PrjColClass::ORIGINAL:
                /// @todo [urgent] get rid of origColumn
                return brushLineEnds(obj->origColumn());
            case PrjColClass::REFERENCE:
                return brushLineEnds(fly.getRef(*obj));
            case PrjColClass::TRANSLATION:
                return brushLineEnds(fly.getTransl(*obj));
            }
            UNREACHABLE
        }
    case Qt::BackgroundRole: {
            auto obj = toObj(index);
            switch (colMeanings.safeGetV(index.column(), PrjColClass::DUMMY)) {
            case PrjColClass::ID:
                if (obj->cache.mod.has(tr::Mch::META_ID))
                    return BG_MODIFIED;
                return {};
            case PrjColClass::ORIGINAL:
                if (obj->cache.mod.has(tr::Mch::ORIG))
                    return BG_MODIFIED;
                return {};
            case PrjColClass::TRANSLATION:
                if (obj->cache.mod.has(tr::Mch::TRANSL))
                    return BG_MODIFIED;
                return {};
            case PrjColClass::REFERENCE:  // Reference never modifies
            case PrjColClass::DUMMY:      // Dummy never has BG
                return {};
            }
            UNREACHABLE
        }
    case Qt::ForegroundRole: {
            auto obj = toObj(index);
            switch (colMeanings.safeGetV(index.column(), PrjColClass::DUMMY)) {
            case PrjColClass::TRANSLATION: {
                    auto& cl = fgColors[fly.getTransl(*obj).iFg()];
                    return cl.isValid() ? cl : QVariant();
                }
            case PrjColClass::REFERENCE: {
                    auto& cl = fgColors[fly.getRef(*obj).iFg()];
                    return cl.isValid() ? cl : QVariant();
                }
            case PrjColClass::ID:
            case PrjColClass::ORIGINAL:
            case PrjColClass::DUMMY:      // Dummy never has FG
                return {};
            }
            UNREACHABLE
        }
    default:
        return {};
    }
}


QVariant PrjTreeModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    static constinit const ec::Array<const char*, PrjColClass> colNames {
        "?????", "ID", "Original", "Reference", "Translation" };

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            auto meaning = colMeanings.safeGetV(section, PrjColClass::DUMMY);
            return colNames[meaning];
        }
    }
    return {};
}

namespace {

    const tr::IdLib myIds {
        .filePrefix = FILE_PREFIX,
        .fileSuffix = FILE_SUFFIX,
        .groupPrefix = u8"g",
        .textPrefix = {}
    };

}   // anon namespace


Thing<tr::File> PrjTreeModel::addHostedFile()
{
    auto newId = prj->makeId<tr::ObjType::FILE>(myIds);
    auto newIndex = prj->nChildren();
    beginInsertRows(QModelIndex(), newIndex, newIndex);
    auto file = prj->addFile(newId, tr::Modify::YES);
    endInsertRows();
    return { file, toIndex(file, 0) };
}


Thing<tr::Group> PrjTreeModel::addGroup(
        const std::shared_ptr<tr::VirtualGroup>& parent)
{
    if (!parent)
        return {};
    auto newId = parent->makeId<tr::ObjType::GROUP>(myIds);
    auto newIndex = parent->nChildren();
    beginInsertRows(toIndex(parent, 0), newIndex, newIndex);
    auto group = parent->addGroup(newId, tr::Modify::YES);
    endInsertRows();
    return { group, toIndex(group, 0) };
}


Thing<tr::Text> PrjTreeModel::addText(
        const std::shared_ptr<tr::VirtualGroup>& parent)
{
    if (!parent)
        return {};
    auto newId = parent->makeId<tr::ObjType::TEXT>(myIds);
    auto newIndex = parent->nChildren();
    beginInsertRows(toIndex(parent, 0), newIndex, newIndex);
    auto text = parent->addText(newId, {}, tr::Modify::YES);
    endInsertRows();
    return { text, toIndex(text, 0) };
}


std::shared_ptr<tr::Entity> PrjTreeModel::extract(tr::UiObject* obj)
{
    if (!obj)
        return {};
    // Get parent/check once again
    auto pnt = obj->parent();
    auto pntIndex = toIndex(pnt, 0);
    auto myIndex = obj->cache.index;
    auto that1 = index(myIndex, 0, pntIndex);
    auto obj1 = toObj(that1);
    if (obj != obj1)
        throw std::logic_error("[PrjTreeModel.doDelete] Check failed!");
    beginRemoveRows(pntIndex, myIndex, myIndex);
    auto r = toObj(pntIndex)->extractChild(myIndex, tr::Modify::YES);
    endRemoveRows();
    if (!r)
        throw std::logic_error("[PrjTreeModel.doDelete] Identified as deletable, but did not delete");
    return r;
}


PrjTreeModel::CloneResult PrjTreeModel::doClone(const QModelIndex& index)
{
    if (!index.isValid())
        return { tr::CloneErr::BAD_OBJECT, {}, tr::ObjType::TEXT };
    auto obj = toObj(index);
    auto pnt = obj->parent();
    if (auto res = obj->startCloning(pnt)) {
        auto newIndex = pnt->nChildren();
        beginInsertRows(toIndex(pnt, 0), newIndex, newIndex);
        auto v = res.commit(&myIds, tr::Modify::YES);
        endInsertRows();
        return { tr::CloneErr::OK, toIndex(*v, 0), v->objType() };
    } else {
        return { res.err, {}, tr::ObjType::TEXT };
    }
}


void PrjTreeModel::paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
{
    constexpr auto MASK_SF = QStyle::State_Selected | QStyle::State_HasFocus;
    if ((option.state & MASK_SF) == MASK_SF) {  // Sel+focus → leave only sel
        QStyleOptionViewItem option1 = option;
        option1.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, option1, index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}


auto PrjTreeModel::moveUp(const QModelIndex& index) -> MoveResult
{
    auto child = toObj(index);
    auto parent = child->parent();
    if (!parent->canMoveUp(child))
        return {};
    auto indParent = toIndex(parent, 0);
    if (!beginMoveRows(indParent, index.row(), index.row(), indParent, index.row() - 1))
        return {};
    parent->moveUp(child);
    endMoveRows();
    child->checkCanary();
    auto mainInd = toIndex(child, 0);
    if (child->cache.index == 0) {
        auto parentInd = toIndex(parent, 0);
        if (parentInd.isValid()) {
            return { mainInd, parentInd };
        } else {
            return { mainInd, mainInd };
        }
    } else {
        return { mainInd, toIndex(parent->child(child->cache.index - 1).get(), 0) };
    }
}


QModelIndex PrjTreeModel::clearGroup(tr::UiObject* obj)
{
    if (!obj)
        return {};
    obj->checkCanary();
    auto index = toIndex(obj, 0);
    auto nCh = obj->nChildren();
    if (nCh != 0 && index.isValid()) {
        beginRemoveRows(index, 0, nCh - 1);
        obj->clearChildren();
        obj->doModify(tr::Mch::META);
        endRemoveRows();
    }
    return index;
}


auto PrjTreeModel::moveDown(const QModelIndex& index) -> MoveResult
{
    auto child = toObj(index);
    auto parent = child->parent();
    if (!parent->canMoveDown(child))
        return {};
    auto indParent = toIndex(parent, 0);
    // See doc — both index and index+1 are IMMOBILE
    if (!beginMoveRows(indParent, index.row(), index.row(), indParent, index.row() + 2))
        return {};
    parent->moveDown(child);
    endMoveRows();
    child->checkCanary();
    auto mainInd = toIndex(child, 0);
    if (static_cast<size_t>(child->cache.index) + 1 >= parent->nChildren()) {
        return { mainInd, mainInd };
    } else {
        return { mainInd, toIndex(parent->child(child->cache.index + 1), 0) };
    }
}
