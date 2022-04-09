#pragma once

#include <QAbstractItemModel>


class QAbstractItemView;

namespace qmod {

    /// @return [+] wasSelected
    QModelIndex selectAny(QAbstractItemView* view);
    QModelIndex selectFirstAmbiguous(QAbstractItemView* view);


    enum class SelectEnd {
        FIRST,      ///< Select first thing
        LAST        ///< Select last thing
    };

    ///
    /// \brief The LeafPos class
    ///   Small object that traverses tree LEAVES
    ///   (does not touch intermediate nodes)
    ///
    class LeafPos
    {
    public:
        LeafPos(const QModelIndex& aCurr);
        const QModelIndex& index() const { return fCurr; }
        operator const QModelIndex() const { return fCurr; }
        const QAbstractItemModel* model() const { return fCurr.model(); }

        // Non-destructive functions
        [[nodiscard]] LeafPos prev() const;
        [[nodiscard]] LeafPos next() const;
        bool hasPrev() const { return prev().isValid(); }
        bool hasNext() const { return next().isValid(); }

        // Some info
        int nSiblings() const;
        bool isValid() const { return fCurr.isValid(); }
        explicit operator bool() const { return fCurr.isValid(); }

        // Destructive functions (replace THIS)
        void goUp();
        void goToRow(int row);
        template <SelectEnd End> bool goDown();
        /// @return [+] at least one goDown occured
        template <SelectEnd End> bool goAllWayDown();
        bool goNext();
        bool goPrev();
    private:
        QModelIndex fCurr, fParent;

        template <SelectEnd End>
            static int rowOfEnd(int nChildren);
    };

}   // namespace qmod


///// TEMPLATE IMPLEMENTATIONS /////////////////////////////////////////////////

template <>
inline int qmod::LeafPos::rowOfEnd<qmod::SelectEnd::FIRST>(int)
    { return 0; }


template <>
inline int qmod::LeafPos::rowOfEnd<qmod::SelectEnd::LAST>(int nChildren)
    { return nChildren - 1; }


template <qmod::SelectEnd End>
bool qmod::LeafPos::goDown()
{
    if (!model() || model()->flags(fCurr) & Qt::ItemNeverHasChildren)
        return false;
    auto nChildren = model()->rowCount(fCurr);
    if (nChildren == 0)
        return false;
    auto newRow = rowOfEnd<End>(nChildren);
    fParent = fCurr;
    fCurr = model()->index(newRow, fCurr.column(), fCurr);
    return true;
}


template <qmod::SelectEnd End>
bool qmod::LeafPos::goAllWayDown()
{
    if (!goDown<End>())
        return false;
    while (goDown<End>()) {}
    return true;
}
