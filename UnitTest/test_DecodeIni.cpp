// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"


namespace {

    class MyCb : public decode::IniCallback
    {
    public:
        std::u8string s;

        void onGroup(std::u8string_view) override;
        void onVar(std::u8string_view name, std::u8string_view rawValue) override;
        void onEmptyLine() override;
        void onComment(std::u8string_view x) override;
    };

    void MyCb::onGroup(std::u8string_view x)
    {
        s.append(u8"g:");
        s.append(x);
        s.push_back('\n');
    }

    void MyCb::onVar(std::u8string_view name, std::u8string_view rawValue)
    {
        s.append(u8"v:");
        s.append(name);
        s.push_back('=');
        s.append(rawValue);
        s.push_back('\n');
    }

    void MyCb::onEmptyLine()
    {
        s.append(u8"e\n");
    }

    void MyCb::onComment(std::u8string_view x)
    {
        s.append(u8"c:");
        s.append(x);
        s.push_back('\n');
    }

    std::u8string runTest(std::string_view x)
    {
        MyCb r;
        std::istringstream is(std::string{x});
        decode::ini(is, r);
        return r.s;
    }

}   // anon namespace


///// decode::ini //////////////////////////////////////////////////////////////


///
///  BOM, simple workability
///
TEST (DecodeIni, Simple)
{
    std::string_view ini = "\xEF\xBB\xBF" "Alpha=  Bravo  \n";
    auto r = runTest(ini);
    EXPECT_EQ(u8"v:Alpha=  Bravo  \n", r);
}


///
///  Bad BOM
///
TEST (DecodeIni, BadBom)
{
    std::string_view ini = "\xFF\xFE" "Alpha=  Bravo  ";
    EXPECT_THROW(
        { runTest(ini); },
        std::logic_error);
}


///
///  Empty lines + comments
///
TEST (DecodeIni, Comments)
{
    std::string_view ini =
            "\n"
            "     \n"
            "1=2\n"
            "  \t  ;3=4\n"
            "   \v   \n"
            "5=6\n"
            "  \f#7=8   \n"
            "9=10";
    auto r = runTest(ini);
    EXPECT_EQ(
            u8"e\n"
              "e\n"
              "v:1=2\n"
              "c:3=4\n"
              "e\n"
              "v:5=6\n"
              "c:7=8\n"
              "v:9=10\n", r);
}


TEST (DecodeIni, TrimValues)
{
    std::string_view ini =
            "  1  =  2  \n"         // Trim 1 space
            " \f 3 =  45  \n"       // Same but \f also trims
            "  6\f =  78  \n"       // Same
            "[Gr]\n"                // Some group
            "  9 =10 \n"            // Trims
            "  A= 12 \n"            // Should not trim
            "  B \f= 34 \n";        // Should not trim
    auto r = runTest(ini);
    EXPECT_EQ(
            u8"v:1= 2  \n"
              "v:3= 45  \n"
              "v:6= 78  \n"
              "g:Gr\n"
              "v:9=10 \n"
              "v:A= 12 \n"
              "v:B= 34 \n", r);

}


TEST (DecodeIni, TrimGroup)
{
    std::string ini =
            "[G1]\n"
            "[ G 2\t\f]\n"
            "   [  G3  ]    \n"
            "[G4]-";
    auto r = runTest(ini);
    EXPECT_EQ(
            u8"g:G1\n"
              "g:G 2\n"
              "g:G3\n"
              "g:G4]-\n", r);
}
