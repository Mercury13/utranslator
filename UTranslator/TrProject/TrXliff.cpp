// My header
#include "TrXliff.h"

#include "pugixml.hpp"

namespace {

    class IdObject  // interface
    {
    public:
        virtual std::string fixupId(std::u8string_view id) const = 0;
        virtual std::string stickIds(const std::string& pile, std::u8string_view id) const = 0;
        virtual std::string_view idSeparator() const = 0;
        virtual ~IdObject() = default;
    };

    class IdObjFather : public IdObject
    {
    public:
        IdObjFather(std::string aIdSep) : idSep(aIdSep) {}
        std::string_view idSeparator() const final { return idSep; }
    private:
        std::string idSep;
    };

    class KeepIdObj final : public IdObjFather
    {
    public:
        using IdObjFather::IdObjFather;
        virtual std::string fixupId(std::u8string_view id) const override
            { return std::string{str::toSv(id)}; }
        std::string stickIds(const std::string& pile, std::u8string_view id) const override
            { return str::cat(pile, str::toSv(id)); }
    };

    bool isBadChar(unsigned char c)
    {
        if (c >= 0x80 || std::isalnum(c))
            return false;
        switch (c) {
            case ':':
            case '_':
            case '.':
            case '-':
                return false;
            default:
                return true;
            }
    }

    bool hasBadChars(std::string_view s)
    {
        return std::ranges::any_of(s, isBadChar);
    }

    std::string toNmToken(std::string_view s)
    {
        if (!hasBadChars(s))
            return std::string{s};
        std::string r;
        r.reserve(s.length());
        for (auto c : s) {
                if (isBadChar(c)) {
                        r += '_';
                    } else {
                        r += c;
                    }
            }
        return r;
    }

    class UnderIdObj final : public IdObjFather
    {
    public:
        using IdObjFather::IdObjFather;
        std::string fixupId(std::u8string_view id) const override
            { return toNmToken(str::toSv(id)); }
        std::string stickIds(const std::string& pile, std::u8string_view id) const override
            { return pile + toNmToken(str::toSv(id)); }
    };

    std::unique_ptr<IdObject> getIdObj(const xlf::Sets::Id& aSets)
    {
        switch (aSets.badPolicy) {
        case xlf::BadIdPolicy::KEEP:
            return std::make_unique<KeepIdObj>(aSets.separator);
        case xlf::BadIdPolicy::UNDERSCORE:
            return std::make_unique<UnderIdObj>(aSets.separator);
        }
    }

    class ToXliffWalker : public tr::TraverseListener
    {
    public:
        ToXliffWalker(const IdObject& aIdObj, const xlf::Sets::WriteText& aSets,
                      pugi::xml_node aUpperNode);
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onText(const std::shared_ptr<tr::Text>&) override;
    private:
        const IdObject& idObj;
        const xlf::Sets::WriteText& sets;
        struct Frame {
            pugi::xml_node node;
            std::string idPlusSep;
        };
        std::vector<Frame> stack;
    };

    ToXliffWalker::ToXliffWalker(
            const IdObject& aIdObj, const xlf::Sets::WriteText& aSets,
            pugi::xml_node aUpperNode)
        : idObj(aIdObj), sets(aSets)
    {
        stack.emplace_back(aUpperNode, std::string{});
    }

    void ToXliffWalker::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x)
    {
        auto& bk = stack.back();
        auto myId = idObj.stickIds(bk.idPlusSep, x->id);
        auto hGroup = bk.node.append_child("group");
        hGroup.append_attribute("id") = myId.c_str();
        stack.emplace_back(hGroup, str::cat(myId, idObj.idSeparator()));
    }

    void ToXliffWalker::onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&)
    {
        stack.pop_back();
    }

    bool isCdataChar(char c)
    {
        switch (c) {
            case '<':
            case '>':
            case '\r':
            case '\n':
                return true;
            default:
                return false;
            }
    }

    bool hasCdataChars(std::string_view s)
    {
        return std::ranges::any_of(s, isCdataChar);
    }

    ///  @warning   Need u8string for c_str()
    void writeTextTag(pugi::xml_node hUpper, const char* tagName,
                      const std::u8string& text,
                      bool writeCdata)
    {
        auto hLower = hUpper.append_child(tagName);
        auto s = str::toSv(text);
        if (writeCdata && hasCdataChars(s)) {
                // Data() is c_str() here
                // PugiXML automatically splits CDATA by ]]>
                hLower.append_child(pugi::node_cdata).set_value(s.data());
            } else {
                hLower.append_child(pugi::node_pcdata).set_value(s.data());
            }
    }

    void ToXliffWalker::onText(const std::shared_ptr<tr::Text>& x)
    {
        auto& bk = stack.back();
        auto myId = idObj.stickIds(bk.idPlusSep, x->id);
        // Unit
        auto hUnit = bk.node.append_child("unit");
        hUnit.append_attribute("id") = myId.c_str();
        // Segment (one)
        auto hSegment = hUnit.append_child("segment");
        // Source
        writeTextTag(hSegment, "source", x->tr.original, sets.cdata);
        // Target
        if (sets.translation && x->tr.translation) {
            writeTextTag(hSegment, "target", *x->tr.translation, sets.cdata);
        }
    }

}	// anon namespace


void xlf::exportMe(
        const tr::Project& project,
        const std::filesystem::path& fname,
        Sets sets)
{
    if (!project.info.isTranslation())
        sets.writeText.translation = false;
    pugi::xml_document doc;
    // Head
    auto hRoot = doc.append_child("xliff");
    hRoot.append_attribute("xmlns") = "urn:oasis:names:tc:xliff:document:2.0";
    hRoot.append_attribute("version") = "2.0";
    hRoot.append_attribute("srcLang") = project.info.orig.lang.c_str();
    if (sets.writeText.translation) {
        hRoot.append_attribute("trgLang") = project.info.transl.lang.c_str();
    }
    // Files
    std::unique_ptr<IdObject> idObj = getIdObj(sets.id);
    for (auto& v : project.files) {
        auto hFile = hRoot.append_child("file");
        ToXliffWalker xw(*idObj, sets.writeText, hFile);
        hFile.append_attribute("id") = idObj->fixupId(v->id).c_str();
        v->traverse(xw, tr::WalkOrder::EXACT, tr::EnterMe::NO);
    }
    // Finally!
    auto res = doc.save_file(fname.c_str(), "\t",
            pugi::format_save_file_text | pugi::format_indent);
    if (!res)
        throw std::logic_error("Cannot save file.");
}


namespace {

    struct XliffEntry {
        std::u8string val;
    };

    struct HeteroCmp : public std::hash<std::string_view> {
        using std::hash<std::string_view>::operator ();
        using is_transparent = void;
    };
    using MKeyEntry = std::unordered_map<std::string, XliffEntry>;
    using MFile = std::unordered_map<std::string, MKeyEntry>;

    void recurseXliffVgroup(MKeyEntry& r, pugi::xml_node hNode)
    {
        // Subgroups
        for (auto hGroup : hNode.children("group")) {
            recurseXliffVgroup(r, hGroup);
        }
        // Units
        for (auto hUnit : hNode.children("unit")) {
            std::string_view id = hUnit.attribute("id").as_string();
            if (id.empty())
                continue;
            unsigned nSegs = 0;
            for (auto hSeg : hUnit.children("segment")) {
                ++nSegs;
                if (nSegs > 1) {
                    throw std::logic_error("Segmented XLIFFs are unsupported");
                }
                if (auto hTarget = hSeg.child("target")) {
                    auto& data = r[std::string(id)];
                    data.val = str::toU8sv(hTarget.value());
                }
            }
        }
    }

    MFile createXliffMap(
            const std::filesystem::path& fname,
            std::optional<std::string_view> onlyFname)
    {
        pugi::xml_document doc;
        auto result = doc.load_file(fname.c_str());
        if (!result)
            throw std::logic_error(result.description());
        auto hRoot = doc.root();
        // No version is OK
        std::string_view svVersion = hRoot.attribute("version").as_string("3.0");
        if (svVersion != "2.0")
            throw std::logic_error("Support only XLIFF 2.0.");
        MFile r;
        for (auto hFile : hRoot.children("file")) {
            std::string_view itsId = hFile.attribute("id").as_string();
            if (itsId.empty())
                continue;
            auto& file = r[std::string{itsId}];
            recurseXliffVgroup(file, hFile);
        }
        // If 1 file and 1 file, give one more chance
        if (r.size() == 1 && onlyFname) {
            auto& [firstK, firstV] = *r.begin();
            if (firstK != onlyFname) {
                // Move data to another place
                auto content = std::move(firstV);
                r.clear();
                r[std::string{*onlyFname}] = std::move(firstV);
            }
        }
        return r;
    }

    class FromXliffWalker : public tr::TraverseListener
    {
    public:
        FromXliffWalker(const IdObject& aIdObj, const MKeyEntry& aKe);
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onText(const std::shared_ptr<tr::Text>&) override;
    private:
        struct Frame {
            std::string idPlusSep;
        };
        std::vector<Frame> stack;
        const IdObject& idObj;
        const MKeyEntry& ke;
    };

    FromXliffWalker::FromXliffWalker(const IdObject& aIdObj, const MKeyEntry& aKe)
        : idObj(aIdObj), ke(aKe)
    {
        stack.emplace_back(std::string{});
    }

    void FromXliffWalker::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x)
    {
        auto& bk = stack.back();
        auto myId = idObj.stickIds(bk.idPlusSep, x->id);
        stack.emplace_back(str::cat(myId, idObj.idSeparator()));
    }

    void FromXliffWalker::onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&)
    {
        stack.pop_back();
    }

    void FromXliffWalker::onText(const std::shared_ptr<tr::Text>& x)
    {
        auto& bk = stack.back();
        auto myId = idObj.stickIds(bk.idPlusSep, x->id);
    }

}   // anon namespace


void xlf::translate(
        tr::Project& project,
        const std::filesystem::path& fname,
        const Sets& sets)
{
    auto childId = project.onlyChildId();
    std::optional<std::string> fixChildId;
    std::unique_ptr<IdObject> idObj = getIdObj(sets.id);
    if (childId)
        fixChildId = idObj->fixupId(*childId);
    MFile fm = createXliffMap(fname, fixChildId);
    for (auto& file : project.files) {
        auto itMap = fm.find(idObj->fixupId(file->id));
        if (itMap == fm.end())
            continue;
        FromXliffWalker walker(*idObj, itMap->second);
    }
}
