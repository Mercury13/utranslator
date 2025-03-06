#include <iostream>

#include "u_Args.h"

// Transl
#include "TrProject.h"


using namespace std;

enum : unsigned char {
    EXIT_USAGE = 1,
    EXIT_BAD_CMDLINE = 2,
    EXIT_ERROR = 3,
};

#define ENDL "\n"


void writeUsage()
{
    std::cout << ENDL
                 "UTransCon: console version of UTranslator" ENDL
                 ENDL
                 "USAGE: UTransCon filename.uorig(.ufull) -options" ENDL
                 "  Options:" ENDL
                 "  -build:directory     build L10n resource" ENDL
                 ENDL;
}


int myMain(const Args<char8_t>& args)
{
    if (args.size() == 0) {
        writeUsage();
        return EXIT_USAGE;
    }
    enum : unsigned char { I_START = 1 };

    try {
        std::filesystem::path fname = args[0];
        if (!std::filesystem::exists(fname))
            throw std::logic_error("File " + fname.string() + " not found");

        auto prj = tr::Project::make();
        prj->load(fname);
        std::cout << "Loaded project <" << fname.string() << ">." ENDL;

        bool didSmth = false;

        if (auto dir = args.paramOptDef(u8"-build", u8".", I_START)) {
            std::filesystem::path exportDir = *dir;
            prj->doBuild(exportDir);
            std::cout << "Exported data to <" << exportDir.string() << ">." ENDL;
            didSmth = true;
        }

        if (!didSmth) {
            std::cout << "No actions specified! Maybe you wanted -build?" ENDL;
            return EXIT_BAD_CMDLINE;
        }
        return 0;
    } catch (const std::exception& e) {
        std::cout << "ERR: " << e.what() << '\n';
        return EXIT_ERROR;
    } catch (...) {
        std::cout << "ERR: Unknown error" ENDL;
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
