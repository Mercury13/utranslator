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


bool SimpleModifiable::unmodify()
{
    if (fIsModified) {
        fIsModified = false;
        if (fListener) {
            fListener->unmodify();
        }
        return true;
    } else {
        return false;
    }
}


void SimpleModifiable::setStaticModifyListener(Modifiable* aListener)
{
    // Aliasing ctor here
    fListener = aListener;
}
