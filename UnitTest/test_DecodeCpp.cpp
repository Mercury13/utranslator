#include "gtest/gtest.h"

#include "Decoders.h"


//// Decode C++ ////////////////////////////////////////////////////////////////


TEST (Cpp, Simple)
{
    auto x = decode::cpp(U"alpha");
    EXPECT_EQ(U"alpha", x);
}


TEST (Cpp, Simple1)
{
    auto x = decode::cpp(U"@#$^");
    EXPECT_EQ(U"@#$^", x);
}


TEST (Cpp, SimpleSpaces)
{
    auto x = decode::cpp(U"  alpha  ");
    EXPECT_EQ(U"  alpha  ", x);
}


TEST (Cpp, NormalizeEol)
{
    auto x = decode::cpp(U"alpha\r\nbravo");
    EXPECT_EQ(U"alpha\nbravo", x);
}


TEST (Cpp, Quoted)
{
    auto x = decode::cpp(UR"("alpha")");
    EXPECT_EQ(U"alpha", x);
}


TEST (Cpp, QuotedSpaces)
{
    auto x = decode::cpp(UR"(  "alpha"  )");
    EXPECT_EQ(U"alpha", x);
}


TEST (Cpp, QuotedSpacesU8sv)
{
    auto x = decode::cpp(UR"(  u8"alpha"sv  )");
    EXPECT_EQ(U"alpha", x);
}


TEST (Cpp, LineBreakInside)
{
    auto x = decode::cpp(U"\"alpha\\r\\nbravo\"");
    EXPECT_EQ(U"alpha\n\nbravo", x);
}


///
///  The same, we also test raw strings
///
TEST (Cpp, LineBreakInside2)
{
    EXPECT_STREQ ( R"("alpha\r\nbravo")", "\"alpha\\r\\nbravo\"" );
    auto x = decode::cpp(UR"("alpha\r\nbravo")");
    EXPECT_EQ(U"alpha\n\nbravo", x);
}


TEST (Cpp, TwoQuotes)
{
    auto x = decode::cpp(UR"( "alpha" "bravo"  )");
    EXPECT_EQ(U"alphabravo", x);
}


TEST (Cpp, TwoQuotesAndLineEnd)
{
    auto x = decode::cpp(U"\"alpha\"  \n  \"bravo\"  ");
    EXPECT_EQ(U"alphabravo", x);
}


TEST (Cpp, SmthOutside)
{
    auto x = decode::cpp(UR"(  "alpha"  +  u8"bravo"sv  )");
    EXPECT_EQ(U"alpha+bravo", x);
}


TEST (Cpp, SmthOutside2)
{
    auto x = decode::cpp(UR"(  "alpha"  + 1 - p  u8"bravo"sv  )");
    EXPECT_EQ(U"alpha+ 1 - pbravo", x);
}

TEST (Cpp, Commas)
{
    auto x = decode::cpp(UR"(  "alpha" , + 1 - p,  u8"bravo"sv  ,, . )");
    EXPECT_EQ(U"alpha+ 1 - p,bravo. ", x);
}
