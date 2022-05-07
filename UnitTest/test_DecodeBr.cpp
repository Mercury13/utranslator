// What we test
#include "u_Decoders.h"

// Google test
#include "gtest/gtest.h"


///// Decode <br> //////////////////////////////////////////////////////////////


TEST(Br, Simple)
{
    auto q = decode::htmlBr(L"alpha<br>\nbravo<br>charlie<br />delta<br   >echo<br>");
    EXPECT_EQ(L"alpha<br>\nbravo<br>\ncharlie<br>\ndelta<br>\necho<br>\n", q);
}


///// Decode <p> ///////////////////////////////////////////////////////////////


TEST(HtmlP, Simple)
{
    auto q = decode::htmlP(L"alpha<p>\n\n\n\nbravo<p>charlie<p />delta<p   >echo<p>");
    EXPECT_EQ(L"alpha\n\nbravo\n\ncharlie\n\ndelta\n\necho\n\n", q);
}
