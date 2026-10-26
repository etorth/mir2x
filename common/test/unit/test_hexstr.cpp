#include <stdexcept>
#include <catch2/catch_test_macros.hpp>
#include "hexstr.hpp"

TEST_CASE("to_2comp sign-extends from the top bit of the given hex chars", "[hexstr]")
{
    REQUIRE(hexstr::to_2comp("FFF", 3) == -1);
    REQUIRE(hexstr::to_2comp("800", 3) == -2048);
    REQUIRE(hexstr::to_2comp("7FF", 3) == 2047);
    REQUIRE(hexstr::to_2comp("064", 3) == 100);
    REQUIRE(hexstr::to_2comp("ff", 2) == -1);
    REQUIRE(hexstr::to_2comp("0001002A064", 3) == 0);
    REQUIRE(hexstr::to_2comp("80000000", 8) == INT32_MIN);
    REQUIRE(hexstr::to_2comp("7FFFFFFF", 8) == INT32_MAX);
}

TEST_CASE("to_2comp rejects bad length and non-hex chars", "[hexstr]")
{
    REQUIRE_THROWS_AS(hexstr::to_2comp("FFF", 0), std::invalid_argument);
    REQUIRE_THROWS_AS(hexstr::to_2comp("000000000", 9), std::invalid_argument);
    REQUIRE_THROWS_AS(hexstr::to_2comp("0G0", 3), std::invalid_argument);
    REQUIRE_THROWS_AS(hexstr::to_2comp("FF", 3), std::invalid_argument);
}
