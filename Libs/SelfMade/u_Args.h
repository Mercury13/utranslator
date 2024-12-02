#pragma once

#include <deque>
#include <string>
#include <filesystem>
#include <optional>

#include "u_Vector.h"

#include "Mojibake/mojibake.h"

namespace detail {
    constexpr size_t BUF_SIZE = 1024;
    using ProgBuf = wchar_t[BUF_SIZE];

    ///  Recovers program file with other means if available
    ///  @return  Windows: path to program file, or Ø if smth bad happened
    ///           Other OS: always Ø
    std::wstring_view lowLevelRecoverProg(ProgBuf& buf);
}

enum class ArgsUnicode { INST };

template <class Ch>
class Args
{
private:
    using This = Args<Ch>;
public:
    using Sv = std::basic_string_view<Ch>;
    using Str = std::basic_string<Ch>;
    struct Entry {
        Sv full, key, value;

        Entry() noexcept = default;
        Entry(Sv x);

        operator Sv() const { return full; }
        operator std::filesystem::path() const { return full; }
        /// @return [+] user entered an = sign
        bool hasEq() const { return full.length() != key.length(); }
    };
    using Vec = SafeVector<Entry>;
    using iterator = typename Vec::const_iterator;

    template <class OtherCh>
    inline Args(int argc, const OtherCh* const* argv)
        { assign<OtherCh>(argc, argv); }

    template <class OtherCh>
    inline Args(ArgsUnicode, int argc, const OtherCh* const* argv) {
        if constexpr (std::is_same_v<std::remove_cv_t<OtherCh>, char>) {
            assign<char8_t>(argc, reinterpret_cast<const char8_t* const*>(argv));
        } else {
            assign<OtherCh>(argc, argv);
        }
    }

    size_t size() const { return d.size(); }

    /// @warning  0-based, 0-terminated!
    const Entry& operator[](size_t i) const;
    /// @return program name, as precise as possible
    const std::filesystem::path& prog() const { return pr; }

    iterator begin() { return d.begin(); }
    iterator end() { return d.end(); }

    /// @return [+] Has key, regardless of value
    ///         [0] No key
    const Entry* param(Sv x, size_t start = 0) const;

    /// @return [+] Has key, regardless of value
    bool hasParam(Sv x, size_t start = 0) const { return param(x, start); }

    /// @return [def] Has key w/o value (-key), or no key
    ///         [+] has key w/value (-key:value), even empty (-key:)
    Sv paramDef(Sv x, Sv def, size_t start = 0) const;

    /// @return [def] Has key w/o value (-key)
    ///         [+] has key w/value (-key:value), even empty (-key:)
    ///         [-] no key
    std::optional<Sv> paramOptDef(Sv x, Sv def, size_t start = 0) const;
private:
    Vec d;
    std::filesystem::path pr;
    std::deque<Str> storage;
    const Entry defEntry;

    template <class OtherCh>
    void assign(int argc, const OtherCh* const* argv);

    template <class OtherCh> Sv convertCh(const OtherCh* s);
    Sv convertCh(const Ch* s) { return s; }

    /// Recovers program from system if it’s available by other means
    ///  besides argc/argv
    bool recoverProg();
};

template <class Ch>
inline bool Args<Ch>::recoverProg()
{
#ifdef _WIN32
    detail::ProgBuf buf;
    auto svProg = detail::lowLevelRecoverProg(buf);
    if (!svProg.empty()) {
        pr = std::filesystem::absolute( svProg );
        return true;
    }
#endif
    return false;
}


template <class Ch> Args<Ch>::Entry::Entry(Sv x) : full(x), key(x)
{
    static const Ch goodChars[] { ':', '=', 0 };
    auto iEq = x.find_first_of(goodChars, std::size(goodChars) - 1);
    if (iEq != 0 && iEq != std::string_view::npos) {
        key = x.substr(0, iEq);
        value = x.substr(iEq + 1);
    }
}


template <class Ch> template <class OtherCh>
auto Args<Ch>::convertCh(const OtherCh* s) -> Sv
{
    auto dest = mojibake::toQ<Str>(s);
    auto& place = storage.emplace_back(std::move(dest));
    return place;
}

template <class Ch> template <class OtherCh>
void Args<Ch>::assign(int argc, const OtherCh* const* argv)
{
    if (!recoverProg()) {
        if (argc != 0) {
            pr = argv[0];
        }
    }
    if (argc > 1) {
        int newSz = argc - 1;
        d.reserve(newSz);
        for (int i = 1; i < argc; ++i) {
            auto arg = argv[i];
            d.push_back(convertCh(arg));
        }
    }
}


template<class Ch>
auto Args<Ch>::operator[](size_t i) const -> const Entry&
{
    if (i >= size())
        return defEntry;
    return d[i];
}


template<class Ch>
auto Args<Ch>::param(Sv x, size_t start) const -> const Entry*
{
    for (size_t i = start; i < size(); ++i) {
        if (auto& e = d[i]; e.key == x)
            return &e;
    }
    return nullptr;
}


template<class Ch>
auto Args<Ch>::paramDef(Sv x, Sv def, size_t start) const -> Sv
{
    for (size_t i = start; i < size(); ++i) {
        if (auto& e = d[i]; e.key == x) {
            return e.hasEq() ? e.value : def;
        }
    }
    return def;
}


template<class Ch>
auto Args<Ch>::paramOptDef(Sv x, Sv def, size_t start) const -> std::optional<Sv>
{
    for (size_t i = start; i < size(); ++i) {
        if (auto& e = d[i]; e.key == x) {
            return e.hasEq() ? e.value : def;
        }
    }
    return std::nullopt;
}


extern template class Args<char>;
extern template class Args<wchar_t>;
extern template class Args<char8_t>;

extern template void Args<char8_t>::assign<char>(int argc, const char* const* argv);
extern template void Args<char8_t>::assign<wchar_t>(int argc, const wchar_t* const* argv);
extern template void Args<char8_t>::assign<char8_t>(int argc, const char8_t* const* argv);
