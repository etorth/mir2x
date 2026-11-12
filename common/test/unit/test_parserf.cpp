#include <string>
#include <cstdint>
#include <catch2/catch_test_macros.hpp>
#include "parserf.hpp"

TEST_CASE("valid_* checks the range for any integer type", "[parserf]")
{
    REQUIRE( parserf::valid_price(1));
    REQUIRE( parserf::valid_price(SYS_MAXPRICE));
    REQUIRE(!parserf::valid_price(0));
    REQUIRE(!parserf::valid_price(-1));
    REQUIRE(!parserf::valid_price(SYS_MAXPRICE + 1));
    REQUIRE(!parserf::valid_price(UINT64_MAX));

    REQUIRE( parserf::valid_level(1));
    REQUIRE( parserf::valid_level(SYS_MAXLEVEL));
    REQUIRE(!parserf::valid_level(0));
    REQUIRE(!parserf::valid_level(SYS_MAXLEVEL + 1));
    REQUIRE(!parserf::valid_level(INT64_MIN));

    REQUIRE( parserf::valid_packcount(SYS_INVGRIDMAXHOLD));
    REQUIRE(!parserf::valid_packcount(SYS_INVGRIDMAXHOLD + 1));

    REQUIRE( parserf::valid_itemcount(SYS_MAXITEMCOUNT));
    REQUIRE(!parserf::valid_itemcount(SYS_MAXITEMCOUNT + 1));
    REQUIRE(!parserf::valid_itemcount(0));
}

TEST_CASE("parsing accepts decimal digits of the whole string only", "[parserf]")
{
    REQUIRE(parserf::price("1") == 1);
    REQUIRE(parserf::price("007") == 7);
    REQUIRE(parserf::price(std::to_string(SYS_MAXPRICE)) == SYS_MAXPRICE);

    REQUIRE(!parserf::price(""));
    REQUIRE(!parserf::price("0"));
    REQUIRE(!parserf::price("-1"));
    REQUIRE(!parserf::price("+1"));
    REQUIRE(!parserf::price(" 1"));
    REQUIRE(!parserf::price("1 "));
    REQUIRE(!parserf::price("12abc"));
    REQUIRE(!parserf::price("0x10"));
    REQUIRE(!parserf::price(std::to_string(SYS_MAXPRICE + 1)));
    REQUIRE(!parserf::price("99999999999999999999999")); // overflows uint64_t

    REQUIRE(parserf::level("1024") == SYS_MAXLEVEL);
    REQUIRE(!parserf::level("1025"));

    REQUIRE(parserf::packcount("99") == 99);
    REQUIRE(!parserf::packcount("100"));

    REQUIRE(parserf::itemcount(std::to_string(SYS_MAXITEMCOUNT)) == SYS_MAXITEMCOUNT);
    REQUIRE(!parserf::itemcount(std::to_string(SYS_MAXITEMCOUNT + 1)));
}

TEST_CASE("parsing takes u8 strings", "[parserf]")
{
    REQUIRE(parserf::price(u8"100") == 100);
    REQUIRE(parserf::price(std::u8string(u8"100")) == 100);
    REQUIRE(!parserf::price(u8"一百"));
}

TEST_CASE("total_price checks both values and the product", "[parserf]")
{
    REQUIRE(parserf::total_price(100, 3) == 300);
    REQUIRE(parserf::total_price(SYS_MAXPRICE, 1) == SYS_MAXPRICE);
    REQUIRE(parserf::total_price(SYS_MAXPRICE / SYS_MAXITEMCOUNT, SYS_MAXITEMCOUNT) == SYS_MAXPRICE / SYS_MAXITEMCOUNT * SYS_MAXITEMCOUNT);

    REQUIRE(!parserf::total_price(SYS_MAXPRICE, 2));
    REQUIRE(!parserf::total_price(2, SYS_MAXITEMCOUNT));
    REQUIRE(!parserf::total_price(SYS_MAXPRICE / 2 + 1, 2));
    REQUIRE(!parserf::total_price(0, 1));
    REQUIRE(!parserf::total_price(1, 0));
    REQUIRE(!parserf::total_price(-1, 1));
    REQUIRE(!parserf::total_price(1, SYS_MAXITEMCOUNT + 1));
}
