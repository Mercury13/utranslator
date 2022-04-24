#include <iostream>
#include <span>
#include <deque>

#include "u_Strings.h"
#include "u_Args.h"

// Transl
#include "TrProject.h"
#include "TrFile.h"


using namespace std;

enum {
    EXIT_USAGE = 1,
    EXIT_BAD_CMDLINE = 2,
    EXIT_ERROR = 3,
};


void writeUsage()
{
    std::cout << std::endl
              << "UTransCon: console version of UTranslator" << std::endl
              << std::endl
              << "USAGE: UTransCon filename.uorig(.ufull) -options" << std::endl
              << "  Options:" << std::endl
              << "  -build:directory     build L10n resource" << std::endl
              << std::endl;
}


int myMain(const Args<char8_t>& args)
{
    if (args.size() == 0) {
        writeUsage();
        return EXIT_USAGE;
    }
    enum { I_START = 1 };

    try {
        std::filesystem::path fname = args[0];
        if (!std::filesystem::exists(fname))
            throw std::logic_error("File " + fname.string() + " not found");

        auto prj = tr::Project::make();
        prj->load(fname);
        std::cout << "Loaded project <" << fname.string() << ">." << std::endl;

        bool didSmth = false;

        if (auto dir = args.paramOptDef(u8"-build", u8".", I_START)) {
            std::filesystem::path exportDir = *dir;
            prj->doBuild(exportDir);
            std::cout << "Exported data to <" << exportDir.string() << ">." << std::endl;
            didSmth = true;
        }

        if (!didSmth) {
            std::cout << "No actions specified! Maybe you wanted -build?" << std::endl;
            return EXIT_BAD_CMDLINE;
        }
        return 0;
    } catch (const std::exception& e) {
        std::cout << "ERR: " << e.what() << std::endl;
        return EXIT_ERROR;
    } catch (...) {
        std::cout << "ERR: Unknown error" << std::endl;
        return EXIT_ERROR;
    }
}

#ifdef _WIN32
    // Windows entry point
    int wmain(int argc, const wchar_t** argv)
    {
        Args<char8_t> args(argc, argv);
        return myMain(args);
    }
#else
    // Unix entry point
    int main(int argc, const char** argv)
    {
        Args<char8_t> args(ArgsUnicode::INST, argc, argv);
        return myMain(args);
    }
#endif
