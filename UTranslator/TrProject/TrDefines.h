#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "u_EcArray.h"

namespace pugi {
    class xml_node;
}

namespace tr {
    enum class WalkChannel : unsigned char { ORIGINAL, TRANSLATION };

    ///  Opaque handle, but may be used as interface
    class IconObject
    {
    public:
        virtual const char* iconName() const { return nullptr; }
        virtual ~IconObject() = default;
    };
    ///  Opaque handle for icons
    using HIcon = const IconObject*;
}

template <class T>
concept UpCloneable = requires(T& t) {
    { t.clone() } -> std::convertible_to<std::unique_ptr<T>>;
};

template <UpCloneable T>
struct CloningUptr : public std::unique_ptr<T>
{
private:
    using Super = std::unique_ptr<T>;
public:
    using Super::Super;
    using Super::reset;
    using Super::operator =;    

    CloningUptr(CloningUptr<T>&& x) noexcept
        : Super(std::move(x)) {}
    CloningUptr(std::unique_ptr<T>&& x) noexcept
        : Super(std::move(x)) {}
    CloningUptr<T>& operator = (CloningUptr<T>&& x) noexcept
        { Super::operator = (std::move(x)); return *this; }
    std::unique_ptr<T> upClone() const;
    CloningUptr<T> clone() const { return upClone(); }
};


template <UpCloneable T>
std::unique_ptr<T> CloningUptr<T>::upClone() const
{
    if (*this) {
        return (*this)->clone();
    } else {
        return {};
    }
}


namespace tr {

    ///  Implemented:
    ///   * Original: can fully edit
    ///   * Full translation: counts untranslated strings
    ///  Not implemented:
    ///   * Bilingual: has both original and full translation
    ///   * Patch translation: untranslated strings are not bugs;
    ///      someday so-called “triggers”: we react to some text in original
    ///   * Freestyle translation: original is not *.uorig but
    ///      some other project, for translation of e.g. game
    ///
    DEFINE_ENUM_TYPE_IN_NS(tr, PrjType, unsigned char,
        ORIGINAL, FULL_TRANSL)
    extern const ec::Array<const char*, PrjType> prjTypeNames;

    DEFINE_ENUM_TYPE_IN_NS(tr, PrefixSuffixMode, unsigned char,
        OFF, DFLT)

    struct PrjInfo {
        PrjType type = PrjType::ORIGINAL;
        struct Orig {
            std::string lang;
            std::filesystem::path absPath;
            /// @todo [bilingual] do smth with it, always false right now!
            bool isBilingual = false;
            bool operator == (const Orig& x) const = default;
        } orig;
        struct Ref {
            std::filesystem::path absPath;
            bool operator == (const Ref& x) const = default;
            void clear() { *this = Ref(); }
        } ref;
        struct Transl {
            std::string lang;            
            /// @warning  Pseudo-localization is applicable to full/bilingual only
            struct Pseudoloc {
                PrefixSuffixMode prefixSuffixMode = PrefixSuffixMode::OFF;

                bool operator == (const Pseudoloc& x) const = default;

                static const Pseudoloc OFF, DFLT;

                bool isDefault() const { return static_cast<bool>(prefixSuffixMode); }
                void setDefault(bool x) { prefixSuffixMode = static_cast<PrefixSuffixMode>(x); }
                bool isOn() const { return (*this != OFF); }
            } pseudoloc;
            bool operator == (const Transl& x) const = default;
            void clear() { *this = Transl(); }
        } transl;

        bool operator == (const PrjInfo& x) const = default;

        /// @return [+] can edit original, incl. adding files
        static bool canEditOriginal(PrjType type);
        bool canEditOriginal() const { return canEditOriginal(type); }

        /// @return [+] can have reference channel
        ///      We CANNOT have multi-storey translations EN→RU→UK.
        ///      But Ukrainian is close to Russian, and it’s better to translate
        ///      to Ukrainian from Russian → we may have reference channel.
        ///      Original is EN, translation is UK, reference is RU.
        static bool canHaveReference(PrjType type);
        bool canHaveReference() const { return canHaveReference(type); }

        /// @return [+] can add/remove files, edit orig. settings
        /// @warning  canEditOriginal → canAddFiles
        ///           (the converse is false for freestyle project)
        ///      @todo [freestyle, #12] we’ll have freestyle translation,
        ///                     it can edit files, but cannot edit original
        static bool canAddFiles(PrjType type);
        bool canAddFiles() const { return canAddFiles(type); }

        /// @return [+] Everything translation-related is available,
        ///             incl. two new channels (translation, translator’s comment),
        ///             translation settings…
        static bool isTranslation(PrjType type);
        bool isTranslation() const { return isTranslation(type); }

        /// @return [+] Can comment translations (false for original/bilingual
        static bool isTranslationCommentable(PrjType type) { return !canEditOriginal(type); }
        bool isTranslationCommentable() const { return isTranslationCommentable(type); }

        /// @return [+] is full translation, e.g. empty translation automatically needs attention.
        /// @warning  isFullTranslation → isTranslation
        /// @todo [patch, #23] what to do?
        bool isFullTranslation() const { return isTranslation(); }

        /// @return [+] original path matters
        /// @warning  hasOriginalPath → isTranslation
        ///           the converse is false for freestyle translation,
        ///           bilingual
        static bool hasOriginalPath(PrjType type);
        bool hasOriginalPath() const { return hasOriginalPath(type); }

        /// @return [+] Want pseudo-L10n in some manner
        ///         [-] Original is surely unchanged
        bool wantPseudoLoc() const
            { return isFullTranslation() && transl.pseudoloc.isOn(); }

        /// Turns settings to original’s
        void switchToOriginal(WalkChannel channel);

        /// Switches original and translation
        void switchOriginalAndTranslation(
                const std::filesystem::path& pathToNewOriginal);

        /// @return [+] actually has reference
        bool hasReference() const
        {
            // 1. Can have reference.
            // 2. Bilingual’s translation is a reference unless user specified another one.
            return canHaveReference() &&
                    (orig.isBilingual || !ref.absPath.empty());
        }
    };

//  Project types
//                           Original  Bilingual  Normal   Freestyle
//   (implemented)             YES        no       YES       no
//   canAddFiles               YES        YES      no        YES
//   canEditOriginal           YES        YES      no        no
//   canHaveReference          no         no       YES       no
//   isTranslation             no         YES      YES       YES
//   isTranslationCommentable  no         no       YES       YES    == !canEditOriginal
//   isFullTranslation         no       depends  depends   depends
//   hasOriginalPath           no         no       YES       no
//

}   // namespace tr
