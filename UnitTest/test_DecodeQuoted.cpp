// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"


///// decode::quoted ///////////////////////////////////////////////////////////


///
/// No quotes at all — should not change
///
TEST (DecodeQuoted, Unquoted)
{
    std::u8string_view x = u8"  alpha bravo   ";
    auto r = decode::quoted(x);
    EXPECT_EQ(x, r);
}


///
/// No initial quote — should not change
///
TEST (DecodeQuoted, QuotesInside)
{
    std::u8string_view x = u8"  alpha \"\"bravo\"\" charlie\"   ";
    auto r = decode::quoted(x);
    EXPECT_EQ(x, r);
}


///
/// Correct quoted text
///
TEST (DecodeQuoted, Correct)
{
    std::u8string_view x = u8R"(  " alpha ""bravo"" charlie   "   )";
    auto r = decode::quoted(x);
    EXPECT_EQ(u8R"( alpha "bravo" charlie   )", r);
}


///
/// Correct quoted text
///
TEST (DecodeQuoted, Tail)
{
    std::u8string_view x = u8R"(  " alpha ""bravo"" charlie    )";
    auto r = decode::quoted(x);
    EXPECT_EQ(u8R"( alpha "bravo" charlie    )", r);
}


///
/// Lone quote instead of ""
///
TEST (DecodeQuoted, LoneQuote)
{
    std::u8string_view x = u8R"(  " alpha "bravo "   )";
    auto r = decode::quoted(x);
    EXPECT_EQ(u8R"( alpha "bravo )", r);
}


///
/// Ending ""
///
TEST (DecodeQuoted, EndingDoubleQuote)
{
    std::u8string_view x = u8R"(  " alpha ""   )";
    auto r = decode::quoted(x);
    EXPECT_EQ(u8R"( alpha ")", r);
}
