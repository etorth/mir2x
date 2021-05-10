/*
 * =====================================================================================
 *
 *       Filename: pathf.hpp
 *        Created: 03/28/2017 17:04:54
 *    Description:
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
#include <tuple>
#include <cmath>
#include <array>
#include <vector>
#include <functional>

#include "fsa.h"
#include "stlastar.h"
#include "totype.hpp"
#include "strf.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "protocoldef.hpp"

namespace pathf
{
    inline int nextDirection(int dir, int diff = 1)
    {
        fflassert(directionValid(dir));
        constexpr int dirCount = DIR_END - DIR_BEGIN;
        return DIR_BEGIN + (((((dir - DIR_BEGIN) + diff) % dirCount) + dirCount) % dirCount);
    }

    int getDir4 (int /* x */, int /* y */);
    int getDir8 (int /* x */, int /* y */);
    int getDir16(int /* x */, int /* y */);

    std::tuple<int, int> getDir4Off (int /* dir */, int /* distance */);
    std::tuple<int, int> getDir8Off (int /* dir */, int /* distance */);
    std::tuple<int, int> getDir16Off(int /* dir */, int /* distance */);

    std::tuple<int, int> getDirOff(int /* x */, int /* y */, int /* distance */);
}
