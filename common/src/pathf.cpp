/*
 * =====================================================================================
 *
 *       Filename: pathf.cpp
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

#include <cfloat>
#include "pathf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"

namespace
{
    constexpr int g_dir4Off[][2]
    {
       { 0, -1},
       { 1,  0},
       { 0,  1},
       {-1,  0},
    };

    constexpr double g_dir8Off[][2]
    {
        { 0.000000000000000, -1.000000000000000},
        { 0.707106781186547, -0.707106781186547},
        { 1.000000000000000,  0.000000000000000},
        { 0.707106781186547,  0.707106781186547},
        { 0.000000000000000,  1.000000000000000},
        {-0.707106781186547,  0.707106781186548},
        {-1.000000000000000,  0.000000000000000},
        {-0.707106781186547, -0.707106781186547},
    };

    constexpr double g_dir16Off[][2]
    {
        { 0.000000000000000, -1.000000000000000},
        { 0.382683432365090, -0.923879532511287},
        { 0.707106781186547, -0.707106781186547},
        { 0.923879532511287, -0.382683432365090},
        { 1.000000000000000,  0.000000000000000},
        { 0.923879532511287,  0.382683432365090},
        { 0.707106781186547,  0.707106781186547},
        { 0.382683432365090,  0.923879532511287},
        { 0.000000000000000,  1.000000000000000},
        {-0.382683432365090,  0.923879532511287},
        {-0.707106781186547,  0.707106781186548},
        {-0.923879532511287,  0.382683432365090},
        {-1.000000000000000,  0.000000000000000},
        {-0.923879532511287, -0.382683432365090},
        {-0.707106781186547, -0.707106781186547},
        {-0.382683432365090, -0.923879532511287},
    };

    template<typename T, size_t N> size_t findNearestPoint(const T (&d)[N], int x, int y)
    {
        if(x == 0 && y == 0){
            throw fflerror("invalid direction (x = 0, y = 0)");
        }

        size_t index = -1;
        double distance = DBL_MAX;

        // normalization of (x, y) is optional
        // just for better stablity

        const double len = mathf::LDistance<double>(0, 0, x, y);
        const double xnorm = to_df(x) / len;
        const double ynorm = to_df(y) / len;

        for(size_t i = 0; i < N; ++i){
            if(const double curr = mathf::LDistance2<double>(d[i][0], d[i][1], xnorm, ynorm); distance > curr){
                index = i;
                distance = curr;
            }
        }
        return index;
    }
}

int pathf::getDir4(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir4Off, x, y));
}

int pathf::getDir8(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir8Off, x, y));
}

int pathf::getDir16(int x, int y)
{
    if(x == 0 && y == 0){
        return -1;
    }
    return to_d(findNearestPoint(g_dir16Off, x, y));
}

std::tuple<int, int> pathf::getDir4Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 4){
        return {to_d(std::lround(g_dir4Off[dirIndex][0] * d)), to_d(std::lround(g_dir4Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 4): %d", dirIndex);
}

std::tuple<int, int> pathf::getDir8Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 8){
        return {to_d(std::lround(g_dir8Off[dirIndex][0] * d)), to_d(std::lround(g_dir8Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 8): %d", dirIndex);
}

std::tuple<int, int> pathf::getDir16Off(int dirIndex, int d)
{
    if(dirIndex >= 0 && dirIndex < 16){
        return {to_d(std::lround(g_dir16Off[dirIndex][0] * d)), to_d(std::lround(g_dir16Off[dirIndex][1] * d))};
    }
    throw fflerror("direction index is not in [0, 16): %d", dirIndex);
}

std::tuple<int, int> pathf::getDirOff(int x, int y, int distance)
{
    if(x == 0 && y == 0){
        throw fflerror("invalid direction (x = 0, y = 0)");
    }

    const double r = to_df(distance) / std::sqrt(1.0 * x * x + 1.0 * y * y);
    return
    {
        to_d(std::lround(x * r)),
        to_d(std::lround(y * r)),
    };
}

bool pathf::inDCCastRange(const DCCastRange &r, int x0, int y0, int x1, int y1)
{
    fflassert(r);
    switch(r.type){
        case CRT_DIR:
            {
                const int dx = std::abs(x0 - x1);
                const int dy = std::abs(y0 - y1);

                const int dmax = std::max<int>(dx, dy);
                const int dmin = std::min<int>(dx, dy);

                return false
                    || (dmin == dmax && dmax <= r.distance)
                    || (dmin == 0    && dmax <= r.distance);
            }
        case CRT_LONG:
            {
                return true;
            }
        case CRT_LIMITED:
            {
                return mathf::LDistance2(x0, y0, x1, y1) <= r.distance * r.distance;
            }
        default:
            {
                throw fflreach();
            }
    }
}
