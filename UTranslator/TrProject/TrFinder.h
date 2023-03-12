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

        ///  Makes another result from x, only from non-expired w_p’s
        //void fixupDestructiveTo(Result& x);
    private:
        Vec d;
        struct Sz {
            size_t v = NOT_FOUND;
        };
        std::map<Wp, Sz,  std::owner_less<Wp>> ndx;
    };

    class Finder : public tr::TraverseListener
    {
    public:
        Finder(const tr::FindCriterion& aCrit);
        void onText(const std::shared_ptr<tr::Text>&) override;
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        std::unique_ptr<ts::Result> give() { return std::move(r); }
        bool isEmpty() const { return r->isEmpty(); }
    private:
        const tr::FindCriterion& crit;
        std::unique_ptr<Result> r;
    };

    ///
    /// Criterion: any warning
    ///
    class CritWarning : public tr::FindCriterion
    {
    public:
        CritWarning(std::shared_ptr<tr::Project> x) : project(x) {}
        bool matchText(const tr::Text&) const override;
        std::u8string caption() const override { return u8"Find warnings"; }
    private:
        std::shared_ptr<tr::Project> project;
    };

    ///
    /// Criterion: commented by author
    ///
    class CritCommentedByAuthor : public tr::FindCriterion
    {
    public:
        bool matchText(const tr::Text& x) const override { return match(x); }
        bool matchGroup(const tr::VirtualGroup& x) const override { return match(x); }
        std::u8string caption() const override { return u8"Commented by author"; }
    private:
        std::shared_ptr<tr::Project> project;
        static bool match(const tr::Entity& x) { return !x.comm.authors.empty(); }
    };

    ///
    /// Criterion: commented by translator
    ///
    class CritCommentedByTranslator : public tr::FindCriterion
    {
    public:
        bool matchText(const tr::Text& x) const override { return match(x); }
        bool matchGroup(const tr::VirtualGroup& x) const override { return match(x); }
        std::u8string caption() const override { return u8"Commented by translator"; }
    private:
        std::shared_ptr<tr::Project> project;
        static bool match(const tr::Entity& x) { return !x.comm.translators.empty(); }
    };

    ///
    /// Criterion: mismatching # of lines
    ///
    class CritMismatchNumber : public tr::FindCriterion
    {
    public:
        bool matchText(const tr::Text&) const override;
        std::u8string caption() const override { return u8"Mismatching # of lines"; }
    private:
        std::shared_ptr<tr::Project> project;
        static size_t nLines(std::u8string_view x);
    };

    class CritChangedOriginal : public tr::FindCriterion
    {
    public:
        bool matchText(const tr::Text&) const override;
        std::u8string caption() const override { return u8"Changed original"; }
    private:
        std::shared_ptr<tr::Project> project;
    };

}   // namespace ts
