#include "QModels.h"

///// LeafPos //////////////////////////////////////////////////////////////////


qmod::LeafPos::LeafPos(const QModelIndex& aCurr)
    : fCurr(aCurr),
      fParent(fCurr.model() && fCurr.isValid()
              ? fCurr.model()->parent(aCurr)
              : QModelIndex())
      {}


void qmod::LeafPos::goUp()
{
    auto q = model();
    if (!q)
        return;
    fCurr = fParent;
    fParent = q->parent(fCurr);
}


void qmod::LeafPos::goToRow(int row)
{
    if (!model())
        return;
    fCurr = model()->index(row, fCurr.column(), fParent);
}


int qmod::LeafPos::nSiblings() const
{
    if (!model())
        return 0;
    return model()->rowCount(fParent);
}


qmod::LeafPos qmod::LeafPos::prev() const
{
    // Bad cursor?
    if (!*this)
        return *this;

    // 1. Go to prev existing
    auto t = *this;
    while (true) {
        // Can go back?
        auto newRw = t.fCurr.row() - 1;
        if (newRw >= 0) {
            t.goToRow(newRw);
            break;
        }
        // Cannot go back — go upper
        t.goUp();
        if (!t)
            return t;
    }

    // 2. Go inside
    t.goAllWayDown<SelectEnd::LAST>();
    return t;
}


qmod::LeafPos qmod::LeafPos::next() const
{
    // Bad cursor?
    if (!*this)
        return *this;

    auto t = *this;
    // 1. Go all way down
    //    (do it in NEXT but not in PREV)
    //    If managed → OK, we have found a new leaf
    if (t.goAllWayDown<SelectEnd::FIRST>())
        return t;

    // 2. Go to next existing
    while (true) {
        // Can go forward?
        auto newRw = t.fCurr.row() + 1;
        auto newN = t.nSiblings();
        if (newRw < newN) {
            t.goToRow(newRw);
            break;
        }
        // Cannot go forward — go upper
        t.goUp();
        if (!t)
            return t;
    }

    // 3. Go inside
    t.goAllWayDown<SelectEnd::FIRST>();
    return t;
}


bool qmod::LeafPos::goNext()
{
    if (auto x = next()) {
        *this = x;
        return true;
    } else {
        return false;
    }
}


bool qmod::LeafPos::goPrev()
{
    if (auto x = prev()) {
        *this = x;
        return true;
    } else {
        return false;
    }
}
