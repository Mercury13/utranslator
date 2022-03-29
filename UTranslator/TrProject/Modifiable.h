#pragma once

#include <memory>
#include <atomic>

class Modifiable    // interface
{
public:
    virtual bool isModified() const = 0;
    /// @return [+] actually modified
    /// @warning  May return always true, even if already modified!
    virtual bool modify() = 0;
    /// @return [+] actually modified
    /// @warning  May return always true, even if no modifications!
    virtual bool unmodify() = 0;
    /// @return [+] actually modified
    virtual bool forceUnmodify() { return unmodify(); }
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



class SimpleModifiable : public Modifiable
{
public:
    void setStaticModifyListener(Modifiable* aListener);
    bool isModified() const override { return fIsModified; }
    bool modify() override;
    bool unmodify() override { return customUnmodify(false); }
    bool forceUnmodify() override { return customUnmodify(true); }
private:
    MovableAtomic<bool> fIsModified = false;
    Modifiable* fListener = nullptr;
    void notify();
    bool customUnmodify(bool notifyIfNothing);
};
