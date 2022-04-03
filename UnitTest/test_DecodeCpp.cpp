#include "gtest/gtest.h"

#include "Decoders.h"


//// Decode C++ ////////////////////////////////////////////////////////////////


TEST (Cpp, Simple)
{
    auto x = decode::cpp("alpha");
    EXPECT_EQ("alpha", x);
}


TEST (Cpp, Simple1)
{
    auto x = decode::cpp("@#$^");
    EXPECT_EQ("@#$^", x);
}


TEST (Cpp, SimpleSpaces)
{
    auto x = decode::cpp("  alpha  ");
    EXPECT_EQ("  alpha  ", x);
}


TEST (Cpp, NormalizeEol)
{
    auto x = decode::cpp("alpha\r\nbravo");
    EXPECT_EQ("alpha\nbravo", x);
}


TEST (Cpp, Quoted)
{
    auto x = decode::cpp(R"("alpha")");
    EXPECT_EQ("alpha", x);
}


TEST (Cpp, QuotedSpaces)
{
    auto x = decode::cpp(R"(  "alpha"  )");
    EXPECT_EQ("alpha", x);
}


TEST (Cpp, QuotedSpacesU8sv)
{
    auto x = decode::cpp(R"(  u8"alpha"sv  )");
    EXPECT_EQ("alpha", x);
}


TEST (Cpp, LineBreakInside)
{
    auto x = decode::cpp("\"alpha\\r\\nbravo\"");
    EXPECT_EQ("alpha\n\nbravo", x);
}


///
///  The same, we also test raw strings
///
TEST (Cpp, LineBreakInside2)
{
    EXPECT_STREQ ( R"("alpha\r\nbravo")", "\"alpha\\r\\nbravo\"" );
    auto x = decode::cpp(R"("alpha\r\nbravo")");
    EXPECT_EQ("alpha\n\nbravo", x);
}


TEST (Cpp, TwoQuotes)
{
    auto x = decode::cpp(R"( "alpha" "bravo"  )");
    EXPECT_EQ("alphabravo", x);
}


TEST (Cpp, TwoQuotesAndLineEnd)
{
    auto x = decode::cpp("\"alpha\"  \n  \"bravo\"  ");
    EXPECT_EQ("alphabravo", x);
}


TEST (Cpp, SmthOutside)
{
    auto x = decode::cpp(R"(  "alpha"  +  u8"bravo"sv  )");
    EXPECT_EQ("alpha+bravo", x);
}


TEST (Cpp, SmthOutside2)
{
    auto x = decode::cpp(R"(  "alpha"  + 1 - p  u8"bravo"sv  )");
    EXPECT_EQ("alpha+ 1 - pbravo", x);
}
