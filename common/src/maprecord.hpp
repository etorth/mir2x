#pragma once
#include <cstdint>
#include <optional>
#include <initializer_list>
#include "strf.hpp"

struct MapRevivePoint
{
    const char8_t *mapName = u8"";
    const int x = -1;
    const int y = -1;

    operator bool () const
    {
        return str_haschar(mapName);
    }
};

struct MapSwitchArea
{
    const int x = -1;
    const int y = -1;
    const int w =  1;
    const int h =  1;

    const char8_t *endName = u8"";
    const int endX = -1;
    const int endY = -1;

    operator bool () const
    {
        return str_haschar(endName);
    }
};

struct MapRecord
{
    const char8_t *name = nullptr;
    const std::optional<uint32_t> bgmID {};
    const std::optional<uint32_t> miniMapID {};

    const MapRevivePoint revivePoint {};
    const std::initializer_list<MapSwitchArea> switchList {};

    operator bool () const
    {
        return str_haschar(name);
    }
};
