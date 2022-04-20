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
    auto r = escape::cppSv(std::u8string_view{data}, 'Q', cache);
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
    auto r = escape::cppSv(x, 'Q', cache);
    EXPECT_EQ(u8"alpha", r);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(r.data(), x.data());
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
    auto r = escape::cppSv(data, 'Q', cache);
    EXPECT_EQ(u8"alpha\\Qbravo\\\\charlie", r);
    EXPECT_TRUE(cache.starts_with(u8"alpha\\Qbravo\\\\charlie"));
    EXPECT_EQ(r.data(), cache.data());
    // Escape2
    r = escape::cppSv(data2, 'Q', cache);
    EXPECT_EQ(u8"a\\Qb", r);
    EXPECT_TRUE(cache.starts_with(u8"a\\Qba\\Qbravo\\\\charlie"));
    EXPECT_EQ(r.data(), cache.data());
}
