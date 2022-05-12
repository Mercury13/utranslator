// My header
#include "TrFile.h"

// PugiXML
#include "pugixml.hpp"
#include "u_XmlUtils.h"

// Libs
#include "u_Strings.h"


using namespace std::string_view_literals;


const tf::DummyProto tf::DummyProto::INST;
const tf::IniProto tf::IniProto::INST;
const tf::UiProto tf::UiProto::INST;
tf::Dummy tf::Dummy::INST;

constinit const tf::FormatProto* const tf::allProtos[I_N] {
    &tf::DummyProto::INST,
    &tf::IniProto::INST,
    &tf::UiProto::INST
};

const tf::FormatProto* const (&tf::allWorkingProtos)[I_N - 1]
        = reinterpret_cast<const FormatProto* const (&)[I_N - 1]>(allProtos[1]);


///// Loader ///////////////////////////////////////////////////////////////////


void tf::Loader::goToGroupAbs(std::span<const std::u8string_view> groupIds)
{
    goToRoot();
    for (auto v : groupIds)
        goToGroupRel(v);
}


void tf::Loader::goToGroupAbs(std::span<const std::u8string> groupIds)
{
    goToRoot();
    for (auto& v : groupIds)
        goToGroupRel(v);
}


void tf::Loader::goToGroupAbs(std::u8string_view groupId)
{
    goToRoot();
    if (!groupId.empty())
        goToGroupRel(groupId);
}

///// TextInfo /////////////////////////////////////////////////////////////////

std::u8string tf::TextInfo::joinGroupId(std::u8string_view sep) const
{
    return joinIdToDepth(sep, actualDepth());
}


std::u8string tf::TextInfo::joinTextId(std::u8string_view sep) const
{
    return joinIdToDepth(sep, ids.size());
}

std::u8string tf::TextInfo::joinIdToDepth(std::u8string_view sep, size_t depth) const
{
    std::u8string s;
    for (size_t i = 0; i < depth; ++i) {
        if (i != 0)
            s.append(sep);
        s.append(ids[i]);
    }
    return s;
}


///// DummyProto ///////////////////////////////////////////////////////////////


std::unique_ptr<tf::FileFormat> tf::DummyProto::make() const
    { return std::make_unique<Dummy>(); }


std::u8string_view tf::DummyProto::locDescription() const
{
    return u8"UTranslator will not build this file.";
}

///// IniProto /////////////////////////////////////////////////////////////////

std::unique_ptr<tf::FileFormat> tf::IniProto::make() const
    { return std::make_unique<Ini>(); }

std::u8string_view tf::IniProto::locDescription() const
{
    return u8"Windows settings file often used for localization."
           "<p>[Group1.Group2]<br>"
           "id1=String 1<br>"
           "id2=String 2";
}

///// Ini //////////////////////////////////////////////////////////////////////


tf::UnifiedSets tf::Ini::unifiedSets() const
{
    return {
        .textFormat = this->textFormat,
        .textEscape = this->textEscape,
        .multitier = this->multitier,
    };
}


void tf::Ini::setUnifiedSets(const tf::UnifiedSets& x)
{
    textFormat = x.textFormat;
    textEscape = x.textEscape;
    multitier = x.multitier;
}


std::string tf::Ini::bannedIdChars() const
{
    std::string r = "[]=";
    if (multitier.separator.length() == 1)
        return r += multitier.separator[0];
    return r;
}


void tf::Ini::doExport(
        Walker& walker,
        const std::filesystem::path&,
        const std::filesystem::path& fname)
{
    std::u8string cache;
    std::ofstream os(fname, std::ios::binary);
    if (textFormat.writeBom)
        os << bom::u8;
    auto eol = textFormat.eol();
    bool isInitial = true;
    while (auto& q = walker.nextText()) {
        if (q.groupChanged()) {
            if (!isInitial)
                os << eol;
            os << '[' << str::toSv(q.joinGroupId(multitier.separator)) << ']' << eol;
        }
        os << str::toSv(q.textId()) << '=';
        textEscape.write(os, q.text, cache);
        os << eol;
        isInitial = false;
    }
}


namespace {

    class IniImporter : public decode::IniCallback {
    public:
        IniImporter(tf::Loader& aLoader, const escape::Text& aSets,
                    const tf::MultitierStyle& aMultitierStyle)
            : loader(aLoader), sets(aSets), multitierStyle(aMultitierStyle) {}
        void onGroup(std::u8string_view name) override;
        void onVar(std::u8string_view name, std::u8string_view rawValue) override;
        void onEmptyLine() override;
        void onComment(std::u8string_view) override;
    private:
        tf::Loader& loader;
        const escape::Text& sets;
        tf::MultitierStyle multitierStyle;
        std::u8string comment;
    };

    void IniImporter::onGroup(std::u8string_view name)
    {
        auto things = str::splitSv(name, multitierStyle.separator);
        loader.goToGroupAbs(things);
        comment.clear();
    }

    void IniImporter::onEmptyLine()
    {
        comment.clear();
    }

    void IniImporter::onComment(std::u8string_view x)
    {
        if (!comment.empty())
            comment += '\n';
        comment += x;
    }

    void IniImporter::onVar(std::u8string_view name, std::u8string_view rawValue)
    {
        auto text = sets.unescape(rawValue);
        loader.addText(name, text, comment);
        comment.clear();
    }

}   // anon namespace


void tf::Ini::doImport(Loader& loader,
              const std::filesystem::path& fname)
{
    std::ifstream is(fname);
    if (!is.is_open()) {
        throw std::runtime_error("Cannot open file " + str::toStr(fname.filename().u8string()));
    }
    IniImporter im(loader, textEscape, multitier);
    decode::ini(is, im);
}



void tf::Ini::save(pugi::xml_node& node) const
    { unifiedSave(node); }


void tf::Ini::load(const pugi::xml_node& node)
    { unifiedLoad(node); }


filedlg::Filter tf::Ini::fileFilter() const
    { return { L"INI files", L"*.ini" }; }


///// UiProto //////////////////////////////////////////////////////////////////


std::unique_ptr<tf::FileFormat> tf::UiProto::make() const
    { return std::make_unique<Ui>(); }


std::u8string_view tf::UiProto::locDescription() const
{
    return u8"Visually-edited Qt form";
}


///// Ui ///////////////////////////////////////////////////////////////////////


namespace {

    /// No "objectId:text", but just "objectId"
    constinit const std::string_view UI_DEF_PROPS[] { "text", "title" };

    //const char* const WIDGET_ELEMS[] = { "widget", "action", nullptr };
    //const char* const ACTION_ELEMS[] = { "action" };

    /// Elements identified as layouts
    constinit const std::string_view UI_LAYOUT_ELEMS[] { "layout", "item" };

    /// Props that are never translated
    constinit const std::string_view UI_UNTRANS_PROPS[] { "styleSheet", "shortcut" };

    struct WidgetPair {
        std::string_view name;
        std::string_view desc;
    };

    template <size_t N>
    inline bool isIn(std::string_view needle, const std::string_view (&strings)[N])
    {
        return std::find(std::begin(strings), std::end(strings), needle)
                != std::end(strings);
    }

    std::string whatIs(std::string_view widgetType)
    {
        constinit static const WidgetPair pairs[] = {
            { "QLabel", "is a static text" },
            { "QWidget", "is a tab" },
            { "QDialog", "Dialog caption" },
            { "QPlainTextEdit", "is a text editor" },
            { "QPushButton", "is a button" }
        };

        if (widgetType.empty())
            return std::string();

        for (auto& v : pairs) {
            if (widgetType == v.name)
                return std::string{ v.desc };
        }

        // Remove Q
        widgetType = str::remainderSv(widgetType, "Q");

        // Split by capital;
        std::string r;
        r.reserve(widgetType.length());
        for (auto c : widgetType) {
            if (c >= 'A' && c <= 'Z') {
                if (!r.empty())
                    r += ' ';
                r += static_cast<char>(c + 32);
            } else {
                r += c;
            }
        }
        switch (r[0]) {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            r = "is an " + r;
            break;
        default:
            r = "is a " + r;
        }
        return r;
    }

    void workOnProperty(
            std::string_view newName,
            pugi::xml_node hProp,
            std::string_view suffix,
            const std::string& comment,
            tf::Loader& loader)
    {
        std::string_view propName = hProp.attribute("name").as_string();
        if (propName.empty())
            return;

        // Is property a string?
        auto hString = hProp.child("string");
        if (!hString)
            return;

        // Get name
        std::string strName { newName };
        strName += suffix;
        if (!isIn(propName, UI_DEF_PROPS)) {
            if (propName == "windowTitle") {
                strName = ":windowTitle";
            } else {
                if (!suffix.empty())
                    return;
                strName += ':';
                strName += propName;
            }
        }

        // Is property inherently untranslatable?
        if (isIn(propName, UI_UNTRANS_PROPS)) {
            return;
        }

        // Is property marked as untranslatable?
        bool notr = hString.attribute("notr").as_bool(false);
        if (notr) {
            return;
        }

        //std::wcout << L"Found " << str::u2w(strName) << L"." << std::endl;
        // Find that string
        auto text = hString.text().as_string();
        loader.addText(str::toU8sv(strName), str::toU8sv(text), str::toU8sv(comment));
    }

    void traverseXmlNormal(pugi::xml_node hSrc, tf::Loader& loader);

    void traverseXmlWidget(pugi::xml_node hSrc, tf::Loader& loader)
    {
        auto newName = hSrc.attribute("name").as_string();

        const char* widgetType = hSrc.attribute("class").as_string();
        std::string whatIsWidget = whatIs(widgetType);

        // Properties?
        for (auto& hProp : hSrc.children("property")) {
            workOnProperty(newName, hProp, "", whatIsWidget, loader);
        }

        for (auto& hProp : hSrc.children("attribute")) {
            workOnProperty(newName, hProp, ":at", whatIsWidget, loader);
        }

        traverseXmlNormal(hSrc, loader);
    }

    void traverseXmlNormal(pugi::xml_node hSrc, tf::Loader& loader)
    {
        for (auto child : hSrc.children()) {
            std::string_view name = child.name();
            if (name == "widget"sv) {
                // Widget — work in root
                loader.goToRoot();
                traverseXmlWidget(child, loader);
            } else if (name == "action"sv) {
                // Action — work in subgroup
                loader.goToGroupAbs(u8"actions");
                traverseXmlWidget(child, loader);
            } else if (isIn(name, UI_LAYOUT_ELEMS)) {
                traverseXmlNormal(child, loader);
            }
        }
    }

}   // anon namespace

filedlg::Filter tf::Ui::fileFilter() const
    { return { L"Qt UI files", L"*.ui" }; }


void tf::Ui::doImport(Loader& loader, const std::filesystem::path& fname)
{
    /// @todo [urgent] Ui::doImport
    pugi::xml_document doc;
    auto result = doc.load_file(fname.c_str());
    xmlThrowIf(result);

    auto hUi = doc.child("ui");
    if (!hUi)
        throw std::logic_error("No <ui> element in source XML");

    traverseXmlNormal(hUi, loader);
}
