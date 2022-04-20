// Google test
#include "gtest/gtest.h"

// What we test
#include "Decoders.h"

///// Escape C++ ///////////////////////////////////////////////////////////////


///
///  Basic workability
///
TEST(EscapeCpp, Basic)
{
    const char8_t* data = u8"alpha" "\n" "bravo" "\\" "charlie";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q');
    EXPECT_EQ(u8"alpha\\Qbravo\\\\charlie", r);
    EXPECT_TRUE(cache.starts_with(u8"alpha\\Qbravo\\\\charlie"));
    EXPECT_EQ(r.data(), cache.data());
}


///
///  What happens if no need to escape
///
TEST(EscapeCpp, NoEscape)
{
    std::u8string_view x = u8"alpha";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    EXPECT_EQ(u8"alpha", r);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(r.data(), x.data());
}


///
///  Should not escape quote
///
TEST(EscapeCpp, Quote)
{
    std::u8string_view x = u8"al\"pha";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    EXPECT_EQ(u8"al\"pha", r);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(r.data(), x.data());
}


///
///  Special branch in code for one space
///
TEST(EscapeCpp, OneSpace)
{
    std::u8string_view x = u8" ";
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    EXPECT_EQ(u8" ", r);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(r.data(), x.data());
}


///
///  Special branch in code for empty string
///
TEST(EscapeCpp, Empty)
{
    std::u8string_view x {};
    std::u8string cache;
    auto r = escape::cppSv(x, cache, 'Q');
    EXPECT_EQ(u8"", r);
    EXPECT_TRUE(cache.empty());
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
    EXPECT_EQ(u8"alpha\\Qbravo\\\\charlie", r);
    EXPECT_TRUE(cache.starts_with(u8"alpha\\Qbravo\\\\charlie"));
    EXPECT_EQ(r.data(), cache.data());
    // Escape2
    r = escape::cppSv(data2, cache, 'Q');
    EXPECT_EQ(u8"a\\Qb", r);
    EXPECT_TRUE(cache.starts_with(u8"a\\Qba\\Qbravo\\\\charlie"));
    EXPECT_EQ(r.data(), cache.data());
}


///
/// Escape spaces: should return as-is
///
TEST(EscapeCpp, SpacesAsIs)
{
    std::u8string_view data = u8"alpha   bravo";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    EXPECT_EQ(u8"alpha   bravo", r);
    EXPECT_EQ(r.data(), data.data());
}


///
/// Escape spaces: normal
///
TEST(EscapeCpp, SpacesNormal)
{
    std::u8string_view data = u8"   alpha   bravo  ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    EXPECT_EQ(u8"\\s  alpha   bravo \\s", r);
    EXPECT_TRUE(cache.starts_with(u8"\\s  alpha   bravo \\s"));
    EXPECT_EQ(r.data(), cache.data());
}

///
/// Escape spaces: leading only
///
TEST(EscapeCpp, SpacesLeading)
{
    std::u8string_view data = u8"  alpha";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    EXPECT_EQ(u8"\\s alpha", r);
    EXPECT_TRUE(cache.starts_with(u8"\\s alpha"));
    EXPECT_EQ(r.data(), cache.data());
}


///
/// Escape spaces: trailing only
///
TEST(EscapeCpp, SpacesTrailing)
{
    std::u8string_view data = u8"alpha   ";
    std::u8string cache;
    auto r = escape::cppSv(data, cache, 'Q', escape::Spaces::YES);
    EXPECT_EQ(u8"alpha  \\s", r);
    EXPECT_TRUE(cache.starts_with(u8"alpha  \\s"));
    EXPECT_EQ(r.data(), cache.data());
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
    EXPECT_EQ(u8"\\s", r);
    EXPECT_TRUE(cache.empty());
}
