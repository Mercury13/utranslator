// My header
#include "TrXliff.h"

#include "pugixml.hpp"

namespace {

    class ToXliffWalker : public tr::TraverseListener
    {
    public:
        ToXliffWalker(pugi::xml_node aUpperNode, const xlf::Sets& aSets);
        void onEnterGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onLeaveGroup(const std::shared_ptr<tr::VirtualGroup>&) override;
        void onText(const std::shared_ptr<tr::Text>&) override;
        std::string fixupId(std::string_view id) const;
        std::string fixupId(std::u8string_view id) const
            { return fixupId(str::toSv(id)); }
    private:
        struct Frame {
            pugi::xml_node node;
            std::string idPlusSep;
        };
        std::vector<Frame> stack;
        const xlf::Sets& sets;

        std::string stickIds(const std::string& pile, std::string_view id);
        std::string stickIds(const std::string& pile, std::u8string_view id)
            { return stickIds(pile, str::toSv(id)); }
    };

    ToXliffWalker::ToXliffWalker(pugi::xml_node aUpperNode, const xlf::Sets& aSets)
        : sets(aSets)
    {
        stack.emplace_back(aUpperNode, std::string{});
    }

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
        /// @todo [urgent, XLIFF] More tokenizations
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

    std::string ToXliffWalker::stickIds(const std::string& pile, std::string_view id)
    {
        switch (sets.badIdPolicy) {
        case xlf::BadIdPolicy::KEEP:
            return str::cat(pile, id);
        case xlf::BadIdPolicy::UNDERSCORE:
            return pile + toNmToken(id);
        }
        __builtin_unreachable();
    }

    std::string ToXliffWalker::fixupId(std::string_view id) const
    {
        switch (sets.badIdPolicy) {
        case xlf::BadIdPolicy::KEEP:
            return std::string{id};
        case xlf::BadIdPolicy::UNDERSCORE:
            return toNmToken(id);
        }
        __builtin_unreachable();
    }

    void ToXliffWalker::onEnterGroup(const std::shared_ptr<tr::VirtualGroup>& x)
    {
        auto& bk = stack.back();
        auto myId = stickIds(bk.idPlusSep, x->id);
        auto hGroup = bk.node.append_child("group");
        hGroup.append_attribute("id") = myId.c_str();
        stack.emplace_back(hGroup, myId + sets.idSeparator);
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
        auto myId = stickIds(bk.idPlusSep, x->id);
        // Unit
        auto hUnit = bk.node.append_child("unit");
        hUnit.append_attribute("id") = myId.c_str();
        // Segment (one)
        auto hSegment = hUnit.append_child("segment");
        // Source
        writeTextTag(hSegment, "source", x->tr.original, sets.writeCdata);
        // Target
        if (sets.writeTranslation && x->tr.translation) {
            writeTextTag(hSegment, "target", *x->tr.translation, sets.writeCdata);
        }
    }

}	// anon namespace


void xlf::exportMe(
        const tr::Project& project,
        const std::filesystem::path& fname,
        Sets& sets)
{
    if (!project.info.isTranslation())
        sets.writeTranslation = false;
    pugi::xml_document doc;
    // Head
    auto hRoot = doc.append_child("xliff");
    hRoot.append_attribute("xmlns") = "urn:oasis:names:tc:xliff:document:2.0";
    hRoot.append_attribute("version") = "2.0";
    hRoot.append_attribute("srcLang") = project.info.orig.lang.c_str();
    if (sets.writeTranslation) {
        hRoot.append_attribute("trgLang") = project.info.transl.lang.c_str();
    }
    // Files
    for (auto& v : project.files) {
        auto hFile = hRoot.append_child("file");
        ToXliffWalker xw(hFile, sets);
        hFile.append_attribute("id") = xw.fixupId(v->id).c_str();
        v->traverse(xw, tr::WalkOrder::EXACT, tr::EnterMe::NO);
    }
    // Finally!
    doc.save_file(fname.c_str(), "\t",
                  pugi::format_save_file_text | pugi::format_indent);
}
