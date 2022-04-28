// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"


///// Decode <p> and <br> //////////////////////////////////////////////////////


TEST(Br, Simple)
{
    auto q = decode::htmlBr(L"alpha<br>\nbravo<br>charlie<br />delta<br   >echo<br>");
    EXPECT_EQ(L"alpha<br>\nbravo<br>\ncharlie<br>\ndelta<br>\necho<br>\n", q);
}
