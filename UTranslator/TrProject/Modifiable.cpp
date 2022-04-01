#include "Modifiable.h"


bool SimpleModifiable::modify()
{
    auto oldState = fState.exchange(ModState::MOD);
    switch (oldState) {
    case ModState::MOD:
        return false;  // do nothing
    case ModState::TEMP:
    case ModState::UNMOD:
        notify(ModState::MOD);
        return true;
    }
    throw std::logic_error("[SimpleModifiable.modify] Strange state");
}


bool SimpleModifiable::customUnmodify(bool notifyIfNothing)
{
    auto oldState = fState.exchange(ModState::UNMOD);
    switch (oldState) {
    case ModState::UNMOD:
        if (notifyIfNothing)
            notify(ModState::UNMOD);
        return false;  // do nothing
    case ModState::TEMP:
    case ModState::MOD:
        notify(ModState::UNMOD);
        return true;
    }
    throw std::logic_error("[SimpleModifiable.customUnmodify] Strange state");
}


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
