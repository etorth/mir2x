/*
 * =====================================================================================
 *
 *       Filename: maprecord.hpp
 *        Created: 08/31/2017 11:12:01
 *  Last Modified: 09/09/2017 00:23:29
 *
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
#include <utility>

struct LinkEntry
{
    int X;
    int Y;

    int W;
    int H;

    int EndX;
    int EndY;

    const char *EndName;

    constexpr LinkEntry(
            int nX = -1,
            int nY = -1,
            int nW = -1,
            int nH = -1,
            int nEndX = -1,
            int nEndY = -1,
            const char *szEndName = u8"")
        : X(nX)
        , Y(nY)
        , W(nW)
        , H(nH)
        , EndX(nEndX)
        , EndY(nEndY)
        , EndName(szEndName ? szEndName : "") {}
};

class MapRecord
{
    public:
        const char *Name;

    public:
        const LinkEntry LinkArray[8];

    public:
        template<typename... U> constexpr MapRecord(
                const char *szName,

                // template pack for LinkEntry array
                // should put at the end of the argument list
                U&&... u)
            : Name(szName ? szName : "")
            , LinkArray { std::forward<U>(u)... }
        {}
};
