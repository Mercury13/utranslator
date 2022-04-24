// My header
#include "u_Args.h"

#if _WIN32
    #include <windows.h>

    std::wstring_view detail::lowLevelRecoverProg(ProgBuf& buf)
    {
        auto sz = GetModuleFileName(nullptr, buf, BUF_SIZE);
        if (sz >= BUF_SIZE)
            return {};
        return { buf, sz };
    }
#else
    std::wstring_view detail::lowLevelRecoverProg(ProgBuf& buf)
    {
        return {};
    }
#endif


template class Args<char>;
template class Args<wchar_t>;
template class Args<char8_t>;

template void Args<char8_t>::assign<char>(int argc, const char* const* argv);
template void Args<char8_t>::assign<wchar_t>(int argc, const wchar_t* const* argv);
template void Args<char8_t>::assign<char8_t>(int argc, const char8_t* const* argv);
