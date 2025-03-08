#pragma once

// STL
#include <optional>
#include <atomic>

// Translation
#include "TrDefines.h"

// Libs
#include "function_ref.hpp"

namespace tr {

    /// Forward-declared, let it be improved afterwards
    enum class ObjType : unsigned char;

    /// Modification channel
    enum class Mch : unsigned char {
        META    = 1,
        ID      = 2,
        ORIG    = 4,
        TRANSL  = 8,
        COMMENT = 16,   ///< as we do not show comments in table, let it be…
        META_ID = META | ID,
    };

    class Mod {
    public:
        void clear() { v = 0; }
        void set(Mch ch) { v |= static_cast<int>(ch); }
        bool has(Mch ch) const { return v & static_cast<int>(ch); }
    private:
        unsigned v = 0;
    };

    class CanaryObject
    {
    public:
        /// Ctors and op= do nothing: canary depends on pointer
        CanaryObject();
        CanaryObject(const CanaryObject&) : CanaryObject() {}
        CanaryObject& operator=(const CanaryObject&) { return *this; }
        /// No virtual dtor here!
        ~CanaryObject();
        /// Checks whether canary is OK
        void checkCanary() const;
    protected:
        volatile std::atomic<uint32_t> canary  = 0;
        uint32_t goodCanary() const;
    };

    struct Comments {
        std::u8string importers, authors, translators;

        /// @return author’s comment if it’s present, importer’s otherwise
        ///         (we prioritize: importer’s < author’s)
        std::u8string_view importersOrAuthors() const noexcept
            { return authors.empty() ? importers : authors; }

        /// @return importer’s comment if it’s visible (no author’s)
        ///         e.g. as a search channel
        std::u8string_view importersIfVisible() const noexcept
            { return authors.empty() ? importers : std::u8string_view{}; }

        void removeTranslChannel() { translators.clear(); }
    };

    enum class AttentionMode : unsigned char {
        BACKGROUND,     ///< patch translation only — untouched (grey)
        CALM,           ///< normal (green)
        USER_ATTENTION, ///< attention (yellow)
        AUTO_PROBLEM,   ///< untranslated or changed original
    };
    constexpr auto AttentionMode_N = static_cast<int>(AttentionMode::AUTO_PROBLEM) + 1;

    struct Translatable {
        std::u8string original;     ///< Current original string
        std::optional<std::u8string>
                    knownOriginal,  ///< Known original string we translated (never == original!)
                    reference,      ///< Reference string in other language (see PrjInfo.canHaveReference)
                    translation;    ///< Translation for known original (if present) or original
        bool forceAttention = false;
        bool wasChangedToday = false;  ///<
        std::u8string_view translationSv() const
            { return translation ? *translation : std::u8string_view(); }
        AttentionMode attentionMode(const tr::PrjInfo& prjInfo) const;
        AttentionMode baseAttentionMode(const tr::PrjInfo& prjInfo) const;

        struct Info {
            size_t nCpsOrig = 0, nCpsTransl = 0;
        };
        Info info(const tr::PrjInfo& prjInfo) const;
    };

    enum class StatsMode : unsigned char {
        CACHED,         ///< Use cache if present
        SEMICACHED,     ///< Mine is computed, subobjects cached
        DIRECT          ///< Totally computed, both mine and subobjects
    };

    struct Stats {
        size_t nGroups = 0;  ///< # of groups NOT INCLUDING me
        struct Text {
            size_t nBackground = 0, nCalm = 0, nUserAttention = 0, nAutoProblem = 0,
                   nTranslated = 0, nUntranslated = 0;
            size_t nTotal() const noexcept { return nTranslated + nUntranslated; }
            size_t nTotalAttention() const { return nUserAttention + nAutoProblem; }
            bool operator == (const Text& x) const = default;
        } text;
        bool isGroup = false;

        void clear() { *this = Stats(); }
        Stats& operator += (const Stats& x);
        bool operator == (const Stats& x) const = default;
    };

    enum class ExpandState : unsigned char {
        ALL_COLLAPSED,   ///< it + children of all levels are collapsed
        COLLAPSED,       ///< just it is collapsed
        UNKNOWN,         ///< who knows
        EXPANDED         ///< expanded
    };

    struct Passport {};

    struct Trash {
        struct Line {
            std::vector<std::u8string> idChain;
            Translatable tr;
        };

        SafeVector<Line> data;
        std::unique_ptr<Passport> passport = std::make_unique<Passport>();

        bool isEmpty() const noexcept { return data.empty(); }
        bool hasSmth() const noexcept { return !isEmpty(); }
        size_t size() const noexcept { return data.size(); }
    };

    enum class Modify : unsigned char { NO, YES };

    struct BigStats {
        struct LittleBigStats {
            size_t nStrings = 0;
            size_t nCpsOrig = 0;
            size_t nCpsTransl = 0;

            void add(const Translatable::Info& info);
        } all, transl, untransl, dubious;
    };

    /// @todo [urgent, #70] Text here
    class Text;
    using EvText = tl::function_ref<void(Text&)>;
    using EvCText = tl::function_ref<void(const Text&)>;

    /// @todo [urgent, #70] File here, what to do?
    class File;
    /// @todo [urgent, #70] Project here, what to do?
    class Project;
    /// @todo [urgent, #70] VirtualGroup here, what to do?
    class VirtualGroup;

    template <class T>
    struct Pair {
        std::shared_ptr<T> first, second;

        unsigned size() const;
        Pair() = default;
        Pair(std::shared_ptr<T> only) : first(std::move(only)) {}  ///< implicit OK!
        Pair(std::shared_ptr<T> x, std::shared_ptr<T> y)
            : first(std::move(x)), second(std::move(y)) {}
        /// @return [+] its size is 2  [-] fewer
        bool is2() const { return first && second; }
    };

    enum class CloneErr : unsigned char {
        OK,
        UNCLONEABLE,
        BAD_PARENT,
        BAD_OBJECT      ///< Unused in lib, UI only
    };

    class UiObject;

    struct IdLib {
        std::u8string_view filePrefix;
        std::u8string_view fileSuffix;
        std::u8string_view groupPrefix;
        std::u8string_view textPrefix;
    };

    struct CloneObj {
        class Commitable {
        public:
            virtual std::shared_ptr<UiObject> commit(
                    const IdLib* idlib, Modify wantModify) = 0;
            virtual ~Commitable() = default;
        };

        CloneErr err;
        std::unique_ptr<Commitable> action;
        explicit operator bool() const { return static_cast<bool>(action); }

        std::shared_ptr<UiObject> commit(const IdLib* idlib, Modify wantModify)
        {
            if (action)
                return action->commit(idlib, wantModify);
            return {};
        }
    };

    class TraverseListener {    // interface
    public:
        virtual void onText(const std::shared_ptr<Text>&) = 0;
        virtual void onEnterGroup(const std::shared_ptr<VirtualGroup>&) {}
        virtual void onLeaveGroup(const std::shared_ptr<VirtualGroup>&) {}
    };

    enum class EnterMe : unsigned char { NO, YES };
    enum class CascadeDropCache : unsigned char { NO, YES };

    struct UpdateInfo {
        size_t nAdded = 0;
        struct ByState {
            size_t nTranslated = 0;
            size_t nUntranslated = 0;

            size_t nTotal() const noexcept { return nTranslated + nUntranslated; }
            ByState& operator += (const ByState& x);
            bool operator == (const ByState& x) const noexcept = default;
        } deleted {}, changed {};
        bool isOriginal = false;

        UpdateInfo& operator += (const UpdateInfo& x);
        bool operator == (const UpdateInfo& x) const = default;
        bool hasSmth() const { return (*this != ZERO); }

        static const UpdateInfo ZERO;
    };

    class Entity;

    class UiObject : public CanaryObject
    {
    public:
        struct Cache {
            int index = -1;             ///< index in tree
            Mod mod;
            std::optional<Stats> stats;
            struct TreeUi {
                ExpandState expandState = ExpandState::UNKNOWN;
                std::weak_ptr<UiObject> currObject {};
            } treeUi;
        } cache;
        // Just here we use virtual dtor!
        virtual ~UiObject() = default;

        virtual ObjType objType() const noexcept = 0;
        virtual std::shared_ptr<UiObject> parent() const = 0;
        virtual size_t nChildren() const noexcept = 0;
        virtual std::shared_ptr<Entity> child(size_t i) const = 0;
        virtual std::u8string_view idColumn() const = 0;
        /// @return [+] was changed
        virtual bool setId(std::u8string_view, tr::Modify) { return false; }
        /// Deletes i’th child
        /// @return extracted child
        /// @warning  Because of recache, complexity is O(n)
        virtual std::shared_ptr<Entity> extractChild(size_t, Modify) { return {}; }
        virtual std::shared_ptr<File> file() = 0;
        virtual void traverseTexts(const EvText&) = 0;
        virtual void traverseCTexts(const EvCText&) const = 0;

        /// @return  ptr to comments, or null
        virtual Comments* comments() { return nullptr; }
        /// @return  ptr to original/translation, or null
        virtual Translatable* translatable() { return nullptr; }
        /// @return  ptr to file info, or null
        virtual FileInfo* ownFileInfo() { return nullptr; }
        /// @return  ptr to own file foprmat, or null
        virtual CloningUptr<tf::FileFormat>* ownFileFormat() { return nullptr; }
        /// Goes together with ownFileFormat. Checks which formats are allowed
        virtual const tf::ProtoFilter* allowedFormats() const { return nullptr; }
        /// @return  ptr to project
        virtual std::shared_ptr<Project> project() = 0;
        /// @return  one or two parent groups for “Add group” / “Add string”
        virtual Pair<VirtualGroup> additionParents() = 0;
        /// @param [in]  idlib   [+] ID library; [0] copy ID
        ///              should exist until commit!
        virtual CloneObj startCloning(
                [[maybe_unused]] const std::shared_ptr<UiObject>& parent) const
            { return { CloneErr::UNCLONEABLE, {} }; }
        virtual void traverse(
                TraverseListener& x, tr::WalkOrder order, EnterMe enterMe) = 0;
        virtual std::shared_ptr<VirtualGroup> nearestGroup() = 0;
        virtual HIcon icon() const { return nullptr; }
        virtual void markChildrenAsAddedToday() = 0;

        /// Makes nChildren == 0
        /// @warning For project: clear() makes a brand new project
        ///                       clearChildren() just removes all files
        virtual void clearChildren() = 0;

        /// Gets statistics, can use cache
        /// @warning Should work with nulls instead of some objects!
        virtual const Stats& stats(StatsMode mode, CascadeDropCache cascade);
        UpdateInfo addedInfo(CascadeDropCache cascade);
        UpdateInfo::ByState deletedInfo(CascadeDropCache cascade);

        /// @return self as shared_ptr
        virtual std::shared_ptr<UiObject> selfUi() = 0;

        /// Removes everything related to translation, leaving only original
        ///   (translation, known translation, translator’s comments)
        virtual void removeTranslChannel() = 0;

        void recache();
        void recursiveRecache();
        void recursiveUnmodify();

        // Utils
        /// @return [+] was actually changed
        bool setOriginal(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setAuthorsComment(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setTranslation(std::optional<std::u8string_view> x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool removeKnownOriginal(tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setTranslatorsComment(std::u8string_view x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setIdless(bool x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setOrigPath(const std::filesystem::path& x, tr::Modify wantModify);
        /// @return [+] was actually changed
        bool setTranslPath(const std::filesystem::path& x, tr::Modify wantModify);
        /// @return some ID
        std::u8string makeId(
                std::u8string_view prefix,
                std::u8string_view suffix) const;
        std::u8string makeTextId(const IdLib& idlib) const;
        /// @return  fileInfo, either own or inherited from file
        const FileInfo* inheritedFileInfo() const;

        /// @return  [+] s_p to this  [0] nothing happened
        std::shared_ptr<Entity> extract(Modify wantModify);
        /// Adds statistics about a single object (not children)
        void doModify(Mch ch);
        bool canMoveUp(const UiObject* aChild) const;
        bool canMoveDown(const UiObject* aChild) const;
        bool moveUp(UiObject* aChild);
        bool moveDown(UiObject* aChild);
        void swapChildren(size_t index1, size_t index2);
        BigStats bigStats() const;

        /// Removes everything related to reference translation, leaving only original
        ///   (reference, known reference if I have someday)
        void removeReferenceChannel();

        // Const verions
        const FileInfo* ownFileInfo() const
            { return const_cast<UiObject*>(this)->ownFileInfo(); }
        std::shared_ptr<const File> file() const
            { return const_cast<UiObject*>(this)->file(); }
        std::shared_ptr<const Project> project() const
            { return const_cast<UiObject*>(this)->project(); }
    protected:
        // passkey idiom
        struct PassKey {};

        /// Exchanges child[i] and child[i+1]
        virtual void doSwapChildren(size_t index1, size_t index2) = 0;
        const Stats& resetCacheIf(const Stats& r, CascadeDropCache cascade);
        void cascadeDropStats();
        void uiStealDataFrom(UiObject& x, UiObject* myParent);
    };

}   // namespace tr
