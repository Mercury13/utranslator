#include "TrProject.h"

#include <bit>

///// UiObject /////////////////////////////////////////////////////////////////

tr::UiObject::UiObject()
{
    canary = goodCanary();
}


tr::UiObject::~UiObject()
{
    canary = 0;
}


uint32_t tr::UiObject::goodCanary() const
{
    static constexpr size_t ALIGNMENT = alignof (uintptr_t);
    static constexpr size_t N_BITS = std::countr_zero(ALIGNMENT);
    static constexpr uint32_t SCRAMBLE = 0xC21330A5;
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) >> N_BITS) ^ SCRAMBLE;
}


void tr::UiObject::checkCanary() const
{
    if (canary != goodCanary())
        throw std::logic_error("[UiObject.checkCanary] Canary is dead, pointer to nowhere!");
}
