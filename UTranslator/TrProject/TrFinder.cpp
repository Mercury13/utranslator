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


//void ts::Result::fixupDestructiveTo(Result& x)
//{
//    for (auto& v : d) {
//        if (v.expired())
//            x.add(std::move(v));
//    }
//}


///// Finder ///////////////////////////////////////////////////////////////////


ts::Finder::Finder(const tr::FindCriterion& aCrit)
    : crit(aCrit), r(new ts::Result) {}

void ts::Finder::onText(const std::shared_ptr<tr::Text>& x)
{
    if (crit.matchText(*x))
        r->add(x);
}

void ts::Finder::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x)
{
    if (crit.matchGroup(*x))
        r->add(x);
}


///// Misc search criteria /////////////////////////////////////////////////////


bool ts::CritWarning::matchText(const tr::Text& x) const
{
    return (x.tr.baseAttentionMode(project->info) >= tr::AttentionMode::USER_ATTENTION);
}

size_t ts::CritMismatchNumber::nLines(std::u8string_view x)
{
    if (x.empty())
        return 0;
    return std::count(x.begin(), x.end(), '\n');
}

bool ts::CritMismatchNumber::matchText(const tr::Text& x) const
{
    if (!x.tr.translation)
        return false;
    return (nLines(x.tr.original) != nLines(*x.tr.translation));
}

bool ts::CritChangedUntransl::matchText(const tr::Text& x) const
{
    return (x.tr.baseAttentionMode(project->info) == tr::AttentionMode::AUTO_PROBLEM);
}

bool ts::CritChangedOnly::matchText(const tr::Text& x) const
{
    return (x.tr.translation
            && x.tr.baseAttentionMode(project->info) == tr::AttentionMode::AUTO_PROBLEM);
}

bool ts::CritUntranslated::matchText(const tr::Text& x) const
{
    return !x.tr.translation;
}

bool ts::CritAttention::matchText(const tr::Text& x) const
{
    return x.tr.forceAttention;
}

bool ts::CritChangedToday::matchText(const tr::Text& x) const
{
    return x.tr.wasChangedToday
        && (x.tr.baseAttentionMode(project->info) == tr::AttentionMode::AUTO_PROBLEM);
}
