// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"

namespace {

    void expectUncached(
            const char8_t* exp,
            std::u8string_view r,
            std::u8string_view x,
            const std::u8string& cache)
    {
        EXPECT_EQ(exp, r);
        EXPECT_TRUE(cache.empty());
        EXPECT_EQ(r.data(), x.data());
    }


    void expectConstant(
            const char8_t* exp,
            std::u8string_view r,
            std::u8string_view x,
            const std::u8string& cache)
    {
        EXPECT_EQ(exp, r);
        EXPECT_TRUE(cache.empty());
        if (r.empty()) {
            EXPECT_EQ(nullptr, r.data());
            // Do not test x, it can be both const char* and null
        } else {
            EXPECT_NE(r.data(), x.data());
        }
        EXPECT_NE(r.data(), cache.data());
    }


    void expectCached(
            const char8_t* exp,
            std::u8string_view r,
            const std::u8string& cache)
    {
        EXPECT_EQ(exp, r);
        EXPECT_TRUE(cache.starts_with(exp));
        EXPECT_EQ(r.data(), cache.data());
    }

}   // anon namespace


///// Escape C++ ///////////////////////////////////////////////////////////////

///
///  Basic workability
///
TEST(EscapeCpp, Basic)
{
    const char8_t* data = u8"alpha" "\n" "bravo" "\\" "charlie";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q');
    expectCached(u8"alpha\\Qbravo\\\\charlie", r, cache);
}


///
///  What happens if no need to escape
///
TEST(EscapeCpp, NoEscape)
{
    std::u8string_view x = u8"alpha";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    expectUncached(u8"alpha", r, x, cache);
}


///
///  Should not escape quote
///
TEST(EscapeCpp, Quote)
{
    std::u8string_view x = u8"al\"pha";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    expectUncached(u8"al\"pha", r, x, cache);
}


///
///  Special branch in code for one space
///
TEST(EscapeCpp, OneSpace)
{
    std::u8string_view x = u8" ";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    expectConstant(u8" ", r, x, cache);
}


///
///  Special branch in code for empty string
///
TEST(EscapeCpp, Empty)
{
    std::u8string_view x {};
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    expectConstant(u8"", r, x, cache);
}


///
///  Escape first longer string, then shorter
///
TEST(EscapeCpp, Shorter)
{
    std::u8string_view data = u8"alpha" "\n" "bravo" "\\" "charlie";
    std::u8string_view data2 = u8"a" "\n" "b";
    std::u8string cache;
    // Escape 1
    auto r = escape::cppSv(data, cache, 'Q');
    expectCached(u8"alpha\\Qbravo\\\\charlie", r, cache);
    // Escape2
    r = escape::cppSv(data2, cache, 'Q');
    expectCached(u8"a\\Qb", r, cache);
    // And stronger reqment for cache
    EXPECT_TRUE(cache.starts_with(u8"a\\Qba\\Qbravo\\\\charlie"));
}


///
/// Escape spaces: should return as-is
///
TEST(EscapeCpp, SpacesAsIs)
{
    std::u8string_view data = u8"alpha   bravo";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    expectUncached(u8"alpha   bravo", r, data, cache);
}


///
/// Escape spaces: normal
///
TEST(EscapeCpp, SpacesNormal)
{
    std::u8string_view data = u8"   alpha   bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    expectCached(u8"\\s  alpha   bravo \\s", r, cache);
}

///
/// Escape spaces: leading only
///
TEST(EscapeCpp, SpacesLeading)
{
    std::u8string_view data = u8"  alpha";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    expectCached(u8"\\s alpha", r, cache);
}


///
/// Escape spaces: trailing only
///
TEST(EscapeCpp, SpacesTrailing)
{
    std::u8string_view data = u8"alpha   ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    expectCached(u8"alpha  \\s", r, cache);
}


///
/// Escape spaces: one space
/// (special branch, return predefined string)
///
TEST(EscapeCpp, SpacesOneSpace)
{
    std::u8string_view data = u8" ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    expectConstant(u8"\\s", r, data, cache);
}


///
/// Quotes, empty string
/// (special branch)
///
TEST(EscapeCpp, QuotesEmpty)
{
    std::u8string_view data = u8"";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::NO, Enquote::YES);
    expectConstant(u8"\"\"", r, data, cache);
}


///
/// Quotes, one space
/// (special branch)
///
TEST(EscapeCpp, QuotesOneSpace)
{
    std::u8string_view data = u8" ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::NO, Enquote::YES);
    expectConstant(u8"\" \"", r, data, cache);
}


///
/// Quotes, test
///
TEST(EscapeCpp, QuotesNormal)
{
    std::u8string_view data = u8"  alpha \n bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::NO, Enquote::YES);
    expectCached(u8R"("  alpha \Q bravo  ")", r, cache);
}


///
/// Quotes, test both break and quotes
///
TEST(EscapeCpp, QuotesQuoted)
{
    std::u8string_view data = u8"  \"alpha\" \n bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::NO, Enquote::YES);
    expectCached(u8R"("  \"alpha\" \Q bravo  ")", r, cache);
}


///
/// Quotes, empty string
/// (special branch)
///
TEST(EscapeCpp, QuotesSpacesEmpty)
{
    std::u8string_view data = u8"";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES, Enquote::YES);
    expectConstant(u8"\"\"", r, data, cache);
}


///
/// Quotes, one space
/// (special branch)
///
TEST(EscapeCpp, QuotesSpacesOneSpace)
{
    std::u8string_view data = u8" ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES, Enquote::YES);
    expectConstant(u8R"("\s")", r, data, cache);
}


///
/// Quotes, test
///
TEST(EscapeCpp, QuotesSpacesNormal)
{
    std::u8string_view data = u8"  alpha \n bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES, Enquote::YES);
    expectCached(u8R"("\s alpha \Q bravo \s")", r, cache);
}


///
/// Quotes, test both break and quotes
///
TEST(EscapeCpp, QuotesSpacesQuoted)
{
    std::u8string_view data = u8"  \"alpha\" \n bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES, Enquote::YES);
    expectCached(u8R"("\s \"alpha\" \Q bravo \s")", r, cache);
}
