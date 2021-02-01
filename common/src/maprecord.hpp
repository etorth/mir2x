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
#include <cstdint>
#include <utility>

struct LinkEntry
{
    const int x;
    const int y;

    const int w;
    const int h;

    const int endX;
    const int endY;

    const char8_t *endName;

    constexpr LinkEntry(
            int argX = -1,
            int argY = -1,
            int argW = -1,
            int argH = -1,
            int argEndX = -1,
            int argEndY = -1,
            const char8_t *argEndName = u8"")
        : x(argX)
        , y(argY)
        , w(argW)
        , h(argH)
        , endX(argEndX)
        , endY(argEndY)
        , endName(argEndName ? argEndName : u8"") {}
};

class MapRecord
{
    public:
        const char8_t *name;
        const uint32_t mmapID;

    public:
        const LinkEntry linkArray[8];

    public:
        template<typename... U> constexpr MapRecord(const char8_t *argName, uint32_t argmmapID = 0, U&&... u)
            : name(argName ? argName : u8"")
            , mmapID(argmmapID)
            , linkArray { std::forward<U>(u)... }
        {}
};
