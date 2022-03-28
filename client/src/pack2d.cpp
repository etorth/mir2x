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

void Pack2D::findRoom(PackBin &bin)
{
    validCEx(0, 0, bin.w, bin.h);
    for(int yi = 0; yi <= to_d(m_packMap.size()); ++yi){
        for(int xi = 0; xi + bin.w <= to_d(w()); ++xi){
            if(!occupied(xi, yi, bin.w, bin.h, true)){
                bin.x = xi;
                bin.y = yi;
                return;
            }
        }
    }
    throw fflreach();
}

void Pack2D::pack(std::vector<PackBin> &binList, size_t packWidth, int packMethod)
{
    if(binList.empty()){
        return;
    }

    Pack2D packHelper(packWidth); // assert packWidth
    std::sort(binList.begin(), binList.end(), [packMethod](const auto &x, const auto &y) -> bool
    {
        const auto fnCreateSortHelper = [packMethod](const auto &bin)
        {
            std::array<uint64_t, 3> sortHelper
            {
                to_u64(bin.w * bin.h),
                to_u64(bin.item.itemID),
                to_u64((uintptr_t)(DBCOM_ITEMRECORD(bin.item.itemID).type)),
            };

            std::swap(sortHelper[0], sortHelper[(sortHelper.size() + std::labs(packMethod)) % sortHelper.size()]);
            return sortHelper;
        };

        if(packMethod % 2){
            return fnCreateSortHelper(x) > fnCreateSortHelper(y);
        }
        else{
            return fnCreateSortHelper(x) < fnCreateSortHelper(y);
        }
    });

    for(auto &bin: binList){
        packHelper.findRoom(bin);
        packHelper.occupy(bin.x, bin.y, bin.w, bin.h, true);
    }
}
