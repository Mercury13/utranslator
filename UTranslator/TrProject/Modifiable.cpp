#include "Modifiable.h"


bool SimpleModifiable::changeState(ModState newState, Forced forced)
{
    auto oldState = fState.exchange(newState);
    if (oldState == newState) {   // nothing changed
        if (forced != Forced::NO)
            notify(newState);
        return false;
    } else {    // actually changed
        notify(newState);
        return true;
    }

}


bool SimpleModifiable::modify(Forced forced) { return changeState(ModState::MOD, forced); }
bool SimpleModifiable::unmodify(Forced forced) { return changeState(ModState::UNMOD, forced); }


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
