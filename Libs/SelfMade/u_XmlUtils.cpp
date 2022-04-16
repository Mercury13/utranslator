#include "u_XmlUtils.h"

pugi::xml_node rqChild(pugi::xml_node node, const char* name)
{
    auto child = node.child(name);
    if (!child)
        throw std::logic_error(
                std::string("Tag <") + node.name() + "> needs child <"
                + name + ">");
    return child;
}


pugi::xml_attribute rqAttr(pugi::xml_node node, const char* name)
{
    auto attr = node.attribute(name);
    if (!attr)
        throw std::logic_error(
                std::string("Tag <") + node.name() + "> needs attribute <"
                + name + ">");
    return attr;
}


int parseEnumIntDef(const char* text, int n, const char* const names[], int def)
{
    if (!text)
        return def;
    for (int i = 0; i < n; ++i) {
        if (strcmp(text, names[i]) == 0)
            return i;
    }
    return def;
}


int parseEnumIntRq(const char* text, int n, const char* const names[])
{
    auto val = parseEnumIntDef(text, n, names, -1);
    if (val < 0)
        throw std::logic_error("Unknown name <" + toStr(text) + ">");
    return val;
}
