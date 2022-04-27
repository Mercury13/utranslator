#pragma once

#include <QAbstractButton>

template <class T>
class Zptr {
public:
    constexpr Zptr() noexcept : v(nullptr) {}
    constexpr Zptr(T* x) noexcept : v(x) {}
    constexpr Zptr& operator = (T* x) { v = x; return *this; }
    constexpr T& operator * () const noexcept { return *v; }
    constexpr T* operator-> () const noexcept { return v;  }
    constexpr explicit operator bool() const { return v; }
    constexpr operator T* () const { return v; }
private:
    T* v;
};

class UintRadio
{
public:
    void setRadio(unsigned index, QAbstractButton* button);
    void set(unsigned value);
    void unset();
    unsigned get(unsigned def = 0) const;
    QAbstractButton* buttonAt(unsigned value) const;
private:
    std::vector<Zptr<QAbstractButton>> v;
};

template <class Ec> requires std::is_enum_v<Ec>
class EcRadio : protected UintRadio
{
private:
    using Super = UintRadio;
public:
    void setRadio(Ec index, QAbstractButton* button)
        { Super::setRadio(static_cast<int>(index), button); }
    void set(Ec value) { Super::set(static_cast<int>(value)); }
    using Super::unset;
    Ec get(Ec def = static_cast<Ec>(0)) const
        { return static_cast<Ec>(Super::get(static_cast<int>(def))); }
    QAbstractButton* buttonAt(Ec value) const
        { return Super::buttonAt(static_cast<int>(value)); }
    UintRadio& unified() { return *this; }
    const UintRadio& unified() const { return *this; }
};
