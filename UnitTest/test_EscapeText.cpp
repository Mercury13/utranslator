// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"


TEST (ET_WriteQuoted, Simple)
{
    std::ostringstream os;
    escape::Text::writeQuoted(os, u8"Alpha");
    EXPECT_EQ(R"("Alpha")", os.str());
}


TEST (ET_WriteQuoted, Quoted)
{
    std::ostringstream os;
    escape::Text::writeQuoted(os, u8R"("Alpha" Bravo)");
    EXPECT_EQ(R"("""Alpha"" Bravo")", os.str());
}
