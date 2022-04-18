#pragma once

// C++
#include <cstring>
#include "string.h"

// XML
#include "pugixml.hpp"


template <class T>
inline T rq(T&& val, const char* errmsg)
{
    if (!val) throw std::logic_error(errmsg);
    return std::forward<T>(val);
}

pugi::xml_node rqChild(pugi::xml_node node, const char* name);

/// Need non-null attribute
pugi::xml_attribute rqAttr(pugi::xml_node node, const char* name);

int parseEnumIntDef(const char* text, int n, const char* const names[], int def);

template <class Struc>
int parseEnumIntTechDef(const char* text, int n, const Struc names[], int def)
{
    if (!text)
        return def;
    for (int i = 0; i < n; ++i) {
        if (names[i].techName == text)
            return i;
    }
    return def;
}


inline std::string toStr(const char* text)
    { return text ? std::string{text} : std::string{}; }

int parseEnumIntRq(const char* text, int n, const char* const names[]);

template <class Ec, size_t N> requires std::is_enum_v<Ec>
inline Ec parseEnumDef(const char* text, const char* const (&names)[N], Ec def)
    { return static_cast<Ec>(parseEnumIntDef(
            text, N, names, static_cast<int>(def))); }

template <class Ec, size_t N, class Struc> requires std::is_enum_v<Ec>
inline Ec parseEnumTechDef(const char* text, const Struc (&names)[N], Ec def)
    { return static_cast<Ec>(parseEnumIntTechDef<Struc>(
            text, N, names, static_cast<int>(def))); }

template <class Ec, size_t N> requires std::is_enum_v<Ec>
inline Ec parseEnumRq(const char* text, const char* const (&names)[N])
    { return static_cast<Ec>(parseEnumIntRq(text, N, names)); }
