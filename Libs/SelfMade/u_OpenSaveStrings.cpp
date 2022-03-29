#include "u_OpenSaveStrings.h"

#include "u_Strings.h"

std::wstring filedlg::filterToW32(const Filters& aFilter)
{
    if (aFilter.empty()) // very edge case: return \0 + auto. \0
        return L"\0";
    std::wstring r;
    for (const Filter& f : aFilter) {
        std::wstring fileMaskCode = f.fileMask;
        str::replace(fileMaskCode, L' ', L';');
        r += f.description + L" (" + f.fileMask + L")" + L'\0' + fileMaskCode + L'\0';
    }
    // W32 filter should end with twin 0, one is here, the other is automatic
    return r;
}

std::wstring filedlg::filterToQt(const Filters& aFilter)
{
    if (aFilter.empty()) // very edge case: return nothing
        return {};
    std::wstring r;
    for (const Filter& f : aFilter)
        r += f.description + L" (" + f.fileMask + L");;";
    return r.erase(r.size() - 2); //remove last ";;" for default All files (*)
}

