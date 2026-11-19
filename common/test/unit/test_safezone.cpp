#include <array>
#include <random>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "serdesmsg.hpp"

namespace
{
    // reference: check every grid with contains(), grids out of the map count as in the zone
    std::vector<std::array<int, 2>> bruteForceEdgeGridList(const SDSafeZone &zone, int mapW, int mapH)
    {
        const auto fnInZone = [&zone, mapW, mapH](int x, int y)
        {
            return x < 0 || y < 0 || x >= mapW || y >= mapH || zone.contains(x, y);
        };

        std::vector<std::array<int, 2>> result;
        for(int y = 0; y < mapH; ++y){
            for(int x = 0; x < mapW; ++x){
                if(zone.contains(x, y) && !(fnInZone(x - 1, y) && fnInZone(x + 1, y) && fnInZone(x, y - 1) && fnInZone(x, y + 1))){
                    result.push_back({x, y});
                }
            }
        }
        return result;
    }
}

TEST_CASE("edgeGridList of simple shapes", "[safezone]")
{
    SECTION("a rect gives its border ring")
    {
        const SDSafeZone zone{.rectList{{2, 2, 3, 3}}};
        REQUIRE(zone.edgeGridList(10, 10) == std::vector<std::array<int, 2>>
        {
            {2, 2}, {3, 2}, {4, 2},
            {2, 3},         {4, 3},
            {2, 4}, {3, 4}, {4, 4},
        });
    }

    SECTION("the map border is no edge")
    {
        const SDSafeZone zone{.rectList{{0, 0, 3, 3}}};
        REQUIRE(zone.edgeGridList(10, 10) == std::vector<std::array<int, 2>>
        {
                            {2, 0},
                            {2, 1},
            {0, 2}, {1, 2}, {2, 2},
        });
    }

    SECTION("a zone covering the whole map has no edge")
    {
        REQUIRE(SDSafeZone{.rectList{{0, 0, 10, 10}}}.edgeGridList(10, 10).empty());
        REQUIRE(SDSafeZone{.circleList{{5, 5, 100000}}}.edgeGridList(10, 10).empty());
    }

    SECTION("a circle of r = 1 is a plus sign, its center is inside")
    {
        const SDSafeZone zone{.circleList{{5, 5, 1}}};
        REQUIRE(zone.edgeGridList(10, 10) == std::vector<std::array<int, 2>>
        {
                    {5, 4},
            {4, 5},         {6, 5},
                    {5, 6},
        });
    }

    SECTION("touching shapes merge, the seam is no edge")
    {
        const SDSafeZone zone{.rectList{{2, 2, 2, 1}, {4, 2, 2, 1}}};
        REQUIRE(zone.edgeGridList(10, 10) == SDSafeZone{.rectList{{2, 2, 4, 1}}}.edgeGridList(10, 10));
    }

    SECTION("a zone out of the map has no edge")
    {
        REQUIRE(SDSafeZone{.rectList{{20, 20, 5, 5}}, .circleList{{-30, 5, 10}}}.edgeGridList(10, 10).empty());
    }
}

TEST_CASE("edgeGridList matches checking every grid", "[safezone]")
{
    std::mt19937 rng(20261119);
    const auto fnRand = [&rng](int minVal, int maxVal)
    {
        return std::uniform_int_distribution<int>(minVal, maxVal)(rng);
    };

    for(int round = 0; round < 500; ++round){
        const int mapW = fnRand(1, 60);
        const int mapH = fnRand(1, 60);

        SDSafeZone zone;
        for(int i = fnRand(0, 4); i > 0; --i){
            zone.rectList.push_back({fnRand(-10, mapW + 10), fnRand(-10, mapH + 10), fnRand(1, 30), fnRand(1, 30)});
        }

        for(int i = fnRand(0, 4); i > 0; --i){
            zone.circleList.push_back({fnRand(-10, mapW + 10), fnRand(-10, mapH + 10), fnRand(1, 20)});
        }

        INFO("round " << round << ", map " << mapW << " x " << mapH);
        REQUIRE(zone.edgeGridList(mapW, mapH) == bruteForceEdgeGridList(zone, mapW, mapH));
    }
}
