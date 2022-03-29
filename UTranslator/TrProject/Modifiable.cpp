#include "Modifiable.h"


void SimpleModifiable::modify()
{
    if (!fIsModified) {
        fIsModified = true;
        if (auto q = fListener.lock())
            q->modify();
    }
}


void SimpleModifiable::unmodify()
{
    if (fIsModified) {
        fIsModified = false;
        if (auto q = fListener.lock())
            q->unmodify();
    }
}


void SimpleModifiable::setStaticListener(Modifiable& aListener)
{
    // Aliasing ctor here
    fListener = std::shared_ptr<Modifiable>(std::shared_ptr<int>{}, &aListener);
}
