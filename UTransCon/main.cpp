#include <iostream>
#include <span>
#include <deque>

#include "u_Vector.h"
#include "u_Strings.h"
#include "u_Args.h"

using namespace std;


int myMain(const Args<char8_t>& args)
{
    return 0;
}

#ifdef _WIN32
    int wmain(int argc, const wchar_t** argv)
    {
        Args<char8_t> args(argc, argv);
        return myMain(args);
    }
#else
    #error Someone make an entry point!
#endif
