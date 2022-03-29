#include "Modifiable.h"


bool SimpleModifiable::modify()
{
    if (!fIsModified) {
        fIsModified = true;
        if (fListener) {
            fListener->modify();
        }
        return true;
    } else {
        return false;
    }
}


bool SimpleModifiable::customUnmodify(bool notifyIfNothing)
{
    if (fIsModified) {
        fIsModified = false;
        notify();
        return true;
    } else {
        if (notifyIfNothing)
            notify();
        return false;
    }
}


void SimpleModifiable::notify()
{
    if (fListener) {
        fListener->unmodify();
    }
}


void SimpleModifiable::setStaticModifyListener(Modifiable* aListener)
{
    // Aliasing ctor here
    fListener = aListener;
}
