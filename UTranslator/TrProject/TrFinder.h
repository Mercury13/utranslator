#pragma once

// C++
#include <map>

#include "TrProject.h"
#include "u_Array.h"
#include "u_Vector.h"

namespace ts {  // translation search

    class Result
    {
    public:
        using Sp = std::shared_ptr<tr::UiObject>;
        using Wp = std::weak_ptr<tr::UiObject>;
        using Vec = SafeVector<Wp>;

        using iterator = Vec::const_iterator;

        void add(Wp x);
        size_t find(const Sp& x) const;
        size_t size() const { return d.size(); }
        std::shared_ptr<tr::UiObject> operator [](size_t i) const
            { return d.safeGetV(i, Wp{}).lock(); }
        auto at(size_t i) const { return operator[](i); }
        void clear();
        iterator begin() const { return d.begin(); }
        iterator end() const { return d.end(); }
        bool isEmpty() const { return d.empty(); }

        ///  Makes another result from x, only from non-expored w_p’s
        void fixupDestructiveTo(Result& x);
    private:
        Vec d;
        struct Sz {
            size_t v = NOT_FOUND;
        };
        std::map<Wp, Sz,  std::owner_less<Wp>> ndx;
    };

}   // namespace ts
