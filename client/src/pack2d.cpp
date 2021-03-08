/*
 * =====================================================================================
 *
 *       Filename: pack2d.cpp
 *        Created: 11/07/2017 23:35:04
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

#include <algorithm>
#include "pack2d.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

bool Pack2D::occupied(int x, int y) const
{
    if(!((x >= 0) && (x < to_d(w())) && (y >= 0))){
        throw fflerror("invalid arguments: x = %d, y = %d", x, y);
    }

    if(y >= to_d(m_packMap.size())){
        return false;
    }
    return (bool)(m_packMap[y] & (1 << x));
}

bool Pack2D::occupied(int x, int y, int argW, int argH, bool occupiedAny) const
{
    validCEx(x, y, argW, argH);

    int occupiedCount = 0;
    for(int xi = x; xi < x + argW; ++xi){
        for(int yi = y; yi < y + argH; ++yi){
            if(occupied(xi, yi)){
                occupiedCount++;
            }
        }
    }

    if(occupiedAny){
        return occupiedCount > 0;
    }
    return occupiedCount == argW * argH;
}

void Pack2D::occupy(int x, int y, bool takeIt)
{
    validCEx(x, y, 1, 1);
    if((y >= to_d(m_packMap.size()))){
        m_packMap.resize(y + 1);
    }

    if(takeIt){
        m_packMap[y] |= (1 << x);
    }
    else{
        m_packMap[y] |= (1 << x);
        m_packMap[y] ^= (1 << x);
    }
    shrink();
}

void Pack2D::occupy(int x, int y, int argW, int argH, bool takeIt)
{
    validCEx(x, y, argW, argH);
    for(int xi = x; xi < x + argW; ++xi){
        for(int yi = y; yi < y + argH; ++yi){
            occupy(xi, yi, takeIt);
        }
    }
}

void Pack2D::findRoom(PackBin *binPtr)
{
    if(!binPtr){
        throw fflerror("invalid arguments: binPtr = %p", to_cvptr(binPtr));
    }

    validCEx(0, 0, binPtr->w, binPtr->h);
    for(int yi = 0; yi <= to_d(m_packMap.size()); ++yi){
        for(int xi = 0; xi + binPtr->w <= to_d(w()); ++xi){
            if(!occupied(xi, yi, binPtr->w, binPtr->h, true)){
                binPtr->x = xi;
                binPtr->y = yi;
                return;
            }
        }
    }
    throw bad_reach();
}

void Pack2D::pack(std::vector<PackBin> &binList)
{
    m_packMap.clear();
    if(!binList.empty()){
        add(binList.data(), binList.size());
    }
}

void Pack2D::add(PackBin *binListPtr, size_t binListSize)
{
    if(!(binListPtr && binListSize)){
        throw fflerror("invalid binList: (%p, %zu)", to_cvptr(binListPtr), binListSize);
    }

    std::sort(binListPtr, binListPtr + binListSize, [](const auto &x, const auto &y) -> bool
    {
        return x.w * x.h < y.w * y.h;
    });

    for(size_t i = 0; i < binListSize; ++i){
        findRoom(binListPtr + i);
        occupy(binListPtr[i].x, binListPtr[i].y, binListPtr[i].w, binListPtr[i].h, true);
    }
}
