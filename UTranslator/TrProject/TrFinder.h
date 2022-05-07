#pragma once

// C++
#include <map>

#include "TrProject.h"
#include "u_Array.h"

namespace ts {  // translation search

    class Result
    {
    public:
        size_t find(const std::shared_ptr<tr::UiObject>& x) const;
        size_t count() const;
        std::shared_ptr<tr::UiObject> operator [](size_t x) const;
    private:
        std::vector<std::weak_ptr<tr::UiObject>> results;
        std::map<std::weak_ptr<tr::UiObject>,
                size_t,
                std::owner_less<std::weak_ptr<tr::UiObject>>> ndx;
    };

    Result x;

}
