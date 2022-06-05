#pragma once
#include <cstdint>
#include <optional>
#include <initializer_list>

struct MapSwitch
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
        return endName && endName[0] != '\0';
    }
};

struct MapRecord
{
    const char8_t *name = nullptr;
    const std::optional<uint32_t> bgmID {};
    const std::optional<uint32_t> miniMapID {};
    const std::initializer_list<MapSwitch> mapSwitchList {};

    operator bool () const
    {
        return name && name[0] != '\0';
    }
};
