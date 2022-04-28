// What we test
#include "Decoders.h"

// Testing framework
#include "gtest/gtest.h"


///// DetectBom ////////////////////////////////////////////////////////////////

namespace {

    void run(
            std::string_view input,
            decode::BomType wantedBom,
            std::string_view wantedString)
    {
        std::istringstream is(std::string{input});
        auto r = decode::detectBom(is);
        EXPECT_EQ(wantedBom, r);
        std::string s;
        std::getline(is, s);
        EXPECT_EQ(wantedString, s);
    }

}   // anon namespace


TEST (DetectBom, X)
{
    // No traces of BOM
    run("", decode::BomType::NONE, "");
    run("a", decode::BomType::NONE, "a");
    // Normal UTF-8 BOM
    run("\xEF\xBB\xBF", decode::BomType::UTF8, "");
    run("\xEF\xBB\xBF" "a", decode::BomType::UTF8, "a");
    // Unfinished UTF-8 BOMs
    run("\xEF", decode::BomType::NONE, "\xEF");
    run("\xEF" "a", decode::BomType::NONE, "\xEF" "a");
    run("\xEF\xBB", decode::BomType::NONE, "\xEF\xBB");
    run("\xEF\xBB" "a", decode::BomType::NONE, "\xEF\xBB" "a");
    // Normal UTF-16LE BOM
    run("\xFF\xFE", decode::BomType::UTF16LE, "");
    run("\xFF\xFE" "a", decode::BomType::UTF16LE, "a");
    // Unfinished UTF-16LE BOMs
    run("\xFF", decode::BomType::NONE, "\xFF");
    run("\xFF" "a", decode::BomType::NONE, "\xFF" "a");
}


///
///  Failed somehow → made another test of those one-liners
///
TEST (DetectBom, Utf16be)
{
    // Normal UTF-16BE BOM
    run("\xFE\xFF", decode::BomType::UTF16BE, "");
    run("\xFE\xFF" "a", decode::BomType::UTF16BE, "a");
    // Unfinished UTF-16LE BOMs
    run("\xFE", decode::BomType::NONE, "\xFE");
    run("\xFE" "a", decode::BomType::NONE, "\xFE" "a");
}
