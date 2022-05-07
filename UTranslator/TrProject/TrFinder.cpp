// My header
#include "TrFinder.h"


///// Result ///////////////////////////////////////////////////////////////////


void ts::Result::clear()
{
    d.clear();
    ndx.clear();
}


size_t ts::Result::find(const Sp& x) const
{
    auto it = ndx.find(x);
    if (it == ndx.end())
        return NOT_FOUND;
    return it->second.v;
}


void ts::Result::add(Wp x)
{
    auto& q = ndx[x];
    if (q.v == NOT_FOUND)
        q.v = size();
    d.push_back(std::move(x));
}


void ts::Result::fixupDestructiveTo(Result& x)
{
    for (auto& v : d) {
        if (v.expired())
            x.add(std::move(v));
    }
}
