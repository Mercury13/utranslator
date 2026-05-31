// Google test
#include "gtest/gtest.h"

// What we test
#include "PortableDiff.h"


///// simpleSplit //////////////////////////////////////////////////////////////

TEST (SimpleSplit, Empties)
{
    auto r = qdif::simpleSplit(U"", U"");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, Appear)
{
    auto r = qdif::simpleSplit(U"", U"alpha");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"alpha", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, Disappear)
{
    auto r = qdif::simpleSplit(U"bravo", U"");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"bravo", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

///
///  Also check for greedy front: equal front and equal back
///     → prefer front
///
TEST (SimpleSplit, Equal)
{
    auto r = qdif::simpleSplit(U"alpha", U"alpha");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, AddToFront)
{
    auto r = qdif::simpleSplit(U"alpha", U"bravoalpha");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"bravo", r.bMid);
    EXPECT_EQ(U"alpha", r.commonSuffix);
}

TEST (SimpleSplit, RemoveFromFront)
{
    auto r = qdif::simpleSplit(U"bravoalpha", U"alpha");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"bravo", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"alpha", r.commonSuffix);
}

TEST (SimpleSplit, ChangeFront)
{
    auto r = qdif::simpleSplit(U"charliealpha", U"bravoalpha");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"charlie", r.aMid);
    EXPECT_EQ(U"bravo", r.bMid);
    EXPECT_EQ(U"alpha", r.commonSuffix);
}

///
///  Again greedy front: "a" from "alpha" is front!
///
TEST (SimpleSplit, AddToBack)
{
    auto r = qdif::simpleSplit(U"alpha", U"alphadelta");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"delta", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, RemoveFromBack)
{
    auto r = qdif::simpleSplit(U"alphaecho", U"alpha");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"echo", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, ChangeBack)
{
    auto r = qdif::simpleSplit(U"alphafoxtrot", U"alphagolf");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"foxtrot", r.aMid);
    EXPECT_EQ(U"golf", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

///
///  Adding e.g. "delta" will NOT trigger greedy front,
///    and thus need another trailing letter
///
TEST (SimpleSplit, AddToBoth)
{
    auto r = qdif::simpleSplit(U"alpha", U"hotelalphakilo");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"alpha", r.aMid);
    EXPECT_EQ(U"hotelalphakilo", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, CoincidingBack)
{
    auto r = qdif::simpleSplit(U"alpha", U"hotelalphadelta");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"alph", r.aMid);
    EXPECT_EQ(U"hotelalphadelt", r.bMid);
    EXPECT_EQ(U"a", r.commonSuffix);
}

TEST (SimpleSplit, RemoveFromBoth)
{
    auto r = qdif::simpleSplit(U"indiaalphaecho", U"alpha");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"indiaalphaecho", r.aMid);
    EXPECT_EQ(U"alpha", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, ChangeBoth)
{
    auto r = qdif::simpleSplit(U"julietalphafoxtrot", U"kiloalphagolf");
    EXPECT_EQ(U"", r.commonPrefix);
    EXPECT_EQ(U"julietalphafoxtrot", r.aMid);
    EXPECT_EQ(U"kiloalphagolf", r.bMid);
    EXPECT_EQ(U"", r.commonSuffix);
}

TEST (SimpleSplit, AddToMid)
{
    auto r = qdif::simpleSplit(U"alphabravo", U"alphacharliebravo");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"", r.aMid);
    EXPECT_EQ(U"charlie", r.bMid);
    EXPECT_EQ(U"bravo", r.commonSuffix);
}

TEST (SimpleSplit, RemoveFromMid)
{
    auto r = qdif::simpleSplit(U"alphadeltabravo", U"alphabravo");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"delta", r.aMid);
    EXPECT_EQ(U"", r.bMid);
    EXPECT_EQ(U"bravo", r.commonSuffix);
}

TEST (SimpleSplit, ChangeMid)
{
    auto r = qdif::simpleSplit(U"alphaechobravo", U"alphafoxtrotbravo");
    EXPECT_EQ(U"alpha", r.commonPrefix);
    EXPECT_EQ(U"echo", r.aMid);
    EXPECT_EQ(U"foxtrot", r.bMid);
    EXPECT_EQ(U"bravo", r.commonSuffix);
}

///// editScript ///////////////////////////////////////////////////////////////


TEST(EditScript, Empty)
{
    auto r = qdif::editScript(U"", U"");
    EXPECT_TRUE(r.empty());
}

///
///  In the common span: del only, ins is empty
///
TEST(EditScript, Equal)
{
    auto r = qdif::editScript(U"alpha", U"alpha");
    EXPECT_EQ(1u, r.size());

    auto& r0 = r[0];
    EXPECT_TRUE(r0.isCommon);
    EXPECT_EQ(U"alpha", r0.del);
    EXPECT_EQ(U"", r0.ins);
}

TEST(EditScript, FullChange)
{
    auto r = qdif::editScript(U"alpha", U"bravo");
    EXPECT_EQ(1u, r.size());

    auto& r0 = r[0];
    EXPECT_FALSE(r0.isCommon);
    EXPECT_EQ(U"alpha", r0.del);
    EXPECT_EQ(U"bravo", r0.ins);
}

TEST(EditScript, FullChangeLonger)
{
    auto r = qdif::editScript(U"qvw", U"bravocharlie");
    EXPECT_EQ(1u, r.size());

    auto& r0 = r[0];
    EXPECT_FALSE(r0.isCommon);
    EXPECT_EQ(U"qvw", r0.del);
    EXPECT_EQ(U"bravocharlie", r0.ins);
}

TEST(EditScript, FullChangeShorter)
{
    auto r = qdif::editScript(U"alphadelta", U"qvw");
    EXPECT_EQ(1u, r.size());

    auto& r0 = r[0];
    EXPECT_FALSE(r0.isCommon);
    EXPECT_EQ(U"alphadelta", r0.del);
    EXPECT_EQ(U"qvw", r0.ins);
}

TEST(EditScript, InsFront)
{
    auto r = qdif::editScript(U"alpha", U"qvwalpha");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_FALSE(r0.isCommon);
        EXPECT_EQ(U"", r0.del);
        EXPECT_EQ(U"qvw", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_TRUE(r1.isCommon);
        EXPECT_EQ(U"alpha", r1.del);
        EXPECT_EQ(U"", r1.ins);
    }
}

TEST(EditScript, DelFront)
{
    auto r = qdif::editScript(U"wxyalpha", U"alpha");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_FALSE(r0.isCommon);
        EXPECT_EQ(U"wxy", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_TRUE(r1.isCommon);
        EXPECT_EQ(U"alpha", r1.del);
        EXPECT_EQ(U"", r1.ins);
    }
}

TEST(EditScript, ChangeFront)
{
    auto r = qdif::editScript(U"prsalpha", U"qvwalpha");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_FALSE(r0.isCommon);
        EXPECT_EQ(U"prs", r0.del);
        EXPECT_EQ(U"qvw", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_TRUE(r1.isCommon);
        EXPECT_EQ(U"alpha", r1.del);
        EXPECT_EQ(U"", r1.ins);
    }
}

TEST(EditScript, InsBack)
{
    auto r = qdif::editScript(U"alpha", U"alphaqvw");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"", r1.del);
        EXPECT_EQ(U"qvw", r1.ins);
    }
}

TEST(EditScript, DelBack)
{
    auto r = qdif::editScript(U"alphawxy", U"alpha");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"wxy", r1.del);
        EXPECT_EQ(U"", r1.ins);
    }
}

TEST(EditScript, ChangeBack)
{
    auto r = qdif::editScript(U"alphaprs", U"alphaqvw");
    EXPECT_EQ(2u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"prs", r1.del);
        EXPECT_EQ(U"qvw", r1.ins);
    }
}

TEST(EditScript, InsMid)
{
    auto r = qdif::editScript(U"alphabravo", U"alphaqwxbravo");
    EXPECT_EQ(3u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"", r1.del);
        EXPECT_EQ(U"qwx", r1.ins);
    }

    { auto& r2 = r[2];
        EXPECT_TRUE(r2.isCommon);
        EXPECT_EQ(U"bravo", r2.del);
        EXPECT_EQ(U"", r2.ins);
    }
}

TEST(EditScript, DelMid)
{
    auto r = qdif::editScript(U"alphaqwxbravo", U"alphabravo");
    EXPECT_EQ(3u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"qwx", r1.del);
        EXPECT_EQ(U"", r1.ins);
    }

    { auto& r2 = r[2];
        EXPECT_TRUE(r2.isCommon);
        EXPECT_EQ(U"bravo", r2.del);
        EXPECT_EQ(U"", r2.ins);
    }
}

TEST(EditScript, ChangeMid)
{
    auto r = qdif::editScript(U"alphaqwxbravo", U"alphapstbravo");
    EXPECT_EQ(3u, r.size());

    { auto& r0 = r[0];
        EXPECT_TRUE(r0.isCommon);
        EXPECT_EQ(U"alpha", r0.del);
        EXPECT_EQ(U"", r0.ins);
    }

    { auto& r1 = r[1];
        EXPECT_FALSE(r1.isCommon);
        EXPECT_EQ(U"qwx", r1.del);
        EXPECT_EQ(U"pst", r1.ins);
    }

    { auto& r2 = r[2];
        EXPECT_TRUE(r2.isCommon);
        EXPECT_EQ(U"bravo", r2.del);
        EXPECT_EQ(U"", r2.ins);
    }
}
