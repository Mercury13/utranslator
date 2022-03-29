#ifndef U_OPENSAVESTRINGS_H
#define U_OPENSAVESTRINGS_H

#include <string>
#include "u_Vector.h"

namespace filedlg
{
    ///
    /// @brief The Filter struct
    ///   A single filter consisting of name and mask.
    /// @warning  Everything should be from reliable source; no escaping here!
    ///   At least these chars are banned:
    ///   • parentheses ()
    ///   • semicolon ;
    ///   • all control: \0 etc
    ///   • double space “  ”
    ///   • empty strings “”
    ///
    struct Filter {
        std::wstring description,   ///< “Spreadsheet files”    so no mask
                     fileMask;      ///< “*.csv *.xls *.xlsx”   so space-separated
    };
    using Filters = SafeVector<Filter>;

    std::wstring filterToW32(const Filters& aFilter);
    std::wstring filterToQt(const Filters& aFilter);
}

#endif // U_OPENSAVESTRINGS_H
