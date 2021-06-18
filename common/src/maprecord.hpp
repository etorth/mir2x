/*
 * =====================================================================================
 *
 *       Filename: maprecord.hpp
 *        Created: 08/31/2017 11:12:01
 *    Description: configuration of mir2x map data
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <span>
#include <cstdint>
#include <utility>

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
    const char8_t *name = u8"";
    const uint32_t miniMapID = 0;
    const MapSwitch mapSwitchList[512] = {};

    operator bool () const
    {
        return name && name[0] != '\0';
    }

    auto mapSwitch() const
    {
        for(const auto &p: mapSwitchList){
            if(!p){
                return std::span<const MapSwitch>(mapSwitchList, &p);
            }
        }
        return std::span<const MapSwitch>(mapSwitchList, mapSwitchList + std::extent_v<decltype(mapSwitchList)>);
    }
};
