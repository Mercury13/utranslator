#pragma once

///
///  Translation wrappers on the border of UI and Translation.
///  DOES NOT CONTAIN Qt, L10n
///  But may draw L10n from outside using some function
///

#include "TrProject.h"

namespace tw {

    // Foreground
    enum class Fg { NORMAL, UNTRANSLATED_CAT, ATTENTION, OK, STATS, LIGHT };
    constexpr int Fg_N = static_cast<int>(Fg::LIGHT) + 1;

    /// Localization strings
    struct L10n {
        std::u8string_view untranslated;
        std::u8string_view emptyString;
    };


    class TranslObj {   // interface
    public:
        virtual std::u8string_view str() const = 0;
        virtual bool mayContainEols() const = 0;
        virtual Fg fg() const = 0;

        int iFg() const { return static_cast<int>(fg()); }
    };

    class NoString final : public TranslObj {   // interface
    public:
        std::u8string_view str() const override { return {}; }
        bool mayContainEols() const override { return false; }
        Fg fg() const override { return Fg::NORMAL; }
        static const NoString INST;
    };

    class DumbString : public TranslObj
    {
    public:
        std::u8string_view s {};
        Fg f = Fg::NORMAL;

        // TranslObj
        std::u8string_view str() const override { return s; }
        bool mayContainEols() const override { return true; }
        Fg fg() const override { return f; }

        const DumbString& set(std::u8string_view aS, Fg aFg)
        {
            s = aS;
            f = aFg;
            return *this;
        }
    };

    ///
    /// A collection of flyweight objects for table columns
    ///
    class Flyweight
    {
    public:
        void setL10n(const L10n& x) { l10n = x; }
        const TranslObj& getTransl(tr::UiObject& x);
        const TranslObj& getRef(tr::UiObject& x);
    private:
        L10n l10n;
        DumbString dumb;
        class TranslStats : public TranslObj {
        public:
            tr::Stats stats;
            mutable char cache[50];

            std::u8string_view str() const override;
            bool mayContainEols() const override { return false; }
            Fg fg() const override;
        };
        TranslStats trStats;
    };

}   // namespace tw
