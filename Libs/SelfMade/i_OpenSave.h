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
            const wchar_t* aCaption,    ///< string_view is not null-terminated!
            const Filters& aFilters,
            const wchar_t* aExtension,  ///< string_view is not null-terminated!
            AddToRecent aAddToRecent,
            CheckForAccess aCheckForAccess = CheckForAccess::YES);

    std::wstring save(
            QWidget* aOwner,
            const wchar_t* aCaption,    ///< string_view is not null-terminated!
            const Filters& aFilters,
            const wchar_t* aExtension,  ///< string_view is not null-terminated!
            std::wstring_view aDefaultFname,  ///< Some alchemy here, so OK
            AddToRecent aAddToRecent);

} //filedlg ns


#endif // I_OPENSAVE_H
