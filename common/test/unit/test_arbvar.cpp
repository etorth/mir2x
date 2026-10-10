#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "mathf.hpp"

TEST_CASE("Zero probability counts failures without increasing probability", "[arbvar]")
{
    mathf::ARBVar never(0.0);
    REQUIRE(never.prob() == 0.0);
    REQUIRE(never.count() == 0);
    REQUIRE(never.aprob() == 0.0);
    REQUIRE_FALSE(never.holdroll());
    for(int i = 1; i <= 5; ++i){
        CAPTURE(i);
        REQUIRE_FALSE(never.roll());
        REQUIRE(never.count() == i);
        REQUIRE(never.aprob() == 0.0);
    }
}

TEST_CASE("Certain probability succeeds and resets failure count", "[arbvar]")
{
    mathf::ARBVar always(1.0);
    REQUIRE(always.prob() == 1.0);
    REQUIRE(always.count() == 0);
    REQUIRE(always.aprob() == 1.0);
    for(int i = 0; i < 5; ++i){
        CAPTURE(i);
        REQUIRE(always.holdroll());
        REQUIRE(always.roll());
        REQUIRE(always.count() == 0);
    }
}

TEST_CASE("Half probability uses the adjusted probability table", "[arbvar]")
{
    mathf::ARBVar half(0.5);
    REQUIRE_THAT(half.aprob(), Catch::Matchers::WithinAbs(0.302103027701, 1.0e-12));
    REQUIRE(half.count() == 0);
}
