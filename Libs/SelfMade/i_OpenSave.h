#ifndef I_OPENSAVE_H
#define I_OPENSAVE_H

///
///  Somehow Qt’s open dialog drops cool WinXP+ functionality:
///  for all programs it remembers previous directory.
///  Thus in Windows we reimplement open/save, and in Mac+ use Qt’s.
///

//qt
#include <QWidget>
#include <QDir>
#include <QFileDialog>

//std
#include <string>

//utils
#include "u_OpenSaveStrings.h"


///
///  Simple null-terminated string_view
///
template <class Char, class Traits = std::char_traits<Char>>
class Zsv : public std::basic_string_view<Char, Traits> {
private:
    using Super = std::basic_string_view<Char, Traits>;
public:
    static_assert(sizeof(Char) <= 4);   // our data() work so
    using Super::data;
    using Super::length;
    using Super::empty;
    using Super::substr;

    // Common implicit ctors who are surely null-term
    constexpr Zsv() noexcept = default;
    constexpr Zsv(std::nullptr_t) noexcept : Super() {}
    constexpr Zsv(const Char* x) noexcept : Super(x) {}
    template <class Allocator> constexpr
        Zsv(const std::basic_string<Char,Traits,Allocator>& x) noexcept : Super(x) {}

    // Ctor from s_v is explicit: progger should ensure for himself that string is null-term
    explicit constexpr Zsv(Super x) : Super(x) {}
    explicit constexpr Zsv(const Char* data, size_t len) : Super(data, len) {}

    Zsv& operator = (const Super&) = delete;

    /// @return  nullptr on empty string, and non-null on everything else
    constexpr const Char* c_str() const noexcept { return empty() ? nullptr : Super::data(); }

    /// @return  non-null string always
    const Char* data() const noexcept
        { return empty() ? reinterpret_cast<const Char*>("\0\0\0\0") : Super::data(); }

    /// @return  any data, but if length is 0, cannot access
    constexpr const Char* rawData() const noexcept { return Super::data(); }

    /// String’s tail is surely null-term
    constexpr Zsv substr(size_t pos) const noexcept { return Zsv{ substr(pos) }; }

    // Op QString is really useful :)
    operator QString () const { return QString::fromWCharArray(rawData(), length()); }
private:
    static constexpr const Char sEmpty[] { 0 };
};


namespace filedlg {

    enum class AddToRecent { NO, YES };
    enum class CheckForAccess { NO, YES };

    ///
    /// \brief openDialog
    ///     Runs an “open file” dialog.
    ///     File should exist. Never changes directory.
    /// \attention Do not use in fibers(WinApi function GetOpenFileName crashes then)!!! Maybe solvable, but needs research.
    /// \param aOwner    Owner window
    /// \param aCaption  Dialog caption (or nullptr)
    /// \param aFilter   filedlg::Filters = SafeVector<Filter>
    /// \param aExtension  default extension
    /// \param aAddToRecent  [+] add to recent
    /// \param aCheckForAccess  [+] check file for access
    ///           (uncheck for special files like DBs opened by external DBMS)
    /// \return
    ///     Full file name with
    ///
    std::wstring open(
            QWidget* aOwner,
            Zsv<wchar_t> aCaption,    ///< W32 requires null-termination
            const Filters& aFilters,
            Zsv<wchar_t> aExtension,  ///< W32 requires null-termination
            AddToRecent aAddToRecent,
            CheckForAccess aCheckForAccess = CheckForAccess::YES);

    std::wstring save(
            QWidget* aOwner,
            Zsv<wchar_t> aCaption,    ///< W32 requires null-termination
            const Filters& aFilters,
            Zsv<wchar_t> aExtension,  ///< W32 requires null-termination
            std::wstring_view aDefaultFname,  ///< Some alchemy here, so OK
            AddToRecent aAddToRecent);

} //filedlg ns


#endif // I_OPENSAVE_H
