#include "Modifiable.h"


bool SimpleModifiable::forceState(ModState newState, bool forceNotify)
{
    auto oldState = fState.exchange(newState);
    if (oldState == newState) {   // nothing changed
        if (forceNotify)
            notify(newState);
        return false;
    } else {    // actually changed
        notify(newState);
        return true;
    }

}


bool SimpleModifiable::modify() { return forceState(ModState::MOD, false); }
bool SimpleModifiable::unmodify() { return forceState(ModState::UNMOD, false); }
bool SimpleModifiable::forceUnmodify() { return forceState(ModState::UNMOD, true); }


void SimpleModifiable::notify(ModState state)
{
    if (fListener) {
        fListener->modStateChanged(state);
    }
}


void SimpleModifiable::setStaticModifyListener(ModListener* aListener)
{
    // Aliasing ctor here
    fListener = aListener;
}


bool SimpleModifiable::tempModify()
{
    auto expected = ModState::UNMOD;
    if (fState.compare_exchange_strong(expected, ModState::TEMP)) {
        // Actually was UNMOD
        notify(ModState::MOD);
        return true;
    } else {
        // In other states do nothing
        return false;
    }
}


bool SimpleModifiable::tempRevert()
{
    auto expected = ModState::TEMP;
    if (fState.compare_exchange_strong(expected, ModState::UNMOD)) {
        // Actually was TEMP
        notify(ModState::UNMOD);
        return true;
    } else {
        // In other states do nothing
        return false;
    }
}
