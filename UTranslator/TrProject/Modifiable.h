#pragma once

#include <memory>
#include <atomic>

enum class ModState {
    UNMOD,          ///< Unmodified
    TEMP,           ///< Somewhere in UI or cache there’s a new value that can easily be accepted or reverted
    MOD             ///< MODIFIED
};

class Modifiable    // interface
{
public:
    virtual ModState modState() const noexcept = 0;
    /// @return [+] state changed, even TEMP to MOD
    /// @warning  May return always true, even if already modified!
    virtual bool modify() = 0;
    /// @return [+] state changed
    /// @warning  May return always true, even if no modifications!
    virtual bool unmodify() = 0;
    /// @return [+] state changed
    virtual bool forceUnmodify() { return unmodify(); }
    /// @return [+] state changed
    virtual bool tempModify() { return modify(); }
    /// @return [+] state changed
    virtual bool tempRevert() { return false; }

    // Utils
    bool isModified() const noexcept { return (modState() != ModState::UNMOD); }

    virtual ~Modifiable() = default;
};


template <class T>
class MovableAtomic : public std::atomic<T> {
private:
    using Super = std::atomic<T>;
public:
    MovableAtomic() = default;

    using Super::Super;
    using Super::operator=;

    MovableAtomic(const MovableAtomic& other)
        { this->store(other.load()); }

    MovableAtomic& operator=(const MovableAtomic& other) {
        this->store(other.load());
        return *this;
    }
};


class ModListener {     // interface
public:
    virtual void modStateChanged(ModState state) = 0;
    virtual ~ModListener() = default;
};


class SimpleModifiable : public Modifiable
{
public:
    void setStaticModifyListener(ModListener* aListener);
    ModState modState() const noexcept override { return fState; }
    bool modify() override;
    bool unmodify() override;
    bool forceUnmodify() override;
    bool tempModify() override;
    bool tempRevert() override;
private:
    MovableAtomic<ModState> fState = ModState::UNMOD;
    ModListener* fListener = nullptr;
    void notify(ModState state);
    bool forceState(ModState newState, bool forceNotify);
    bool customUnmodify();
};
