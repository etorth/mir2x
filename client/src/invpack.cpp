/*
 * =====================================================================================
 *
 *       Filename: invpack.cpp
 *        Created: 11/11/2017 01:03:43
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

#include "dbcomid.hpp"
#include "invpack.hpp"
#include "pngtexdb.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "dbcomrecord.hpp"

extern PNGTexDB *g_itemDB;
void InvPack::add(uint32_t itemID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!(ir && count)){
        throw fflerror("invalid arguments: itemID = %llu, count = %zu", to_llu(itemID), count);
    }

    if(!ir.hasDBID()){
        for(auto &bin: m_packBinList){
            if(bin.id == itemID){
                if(bin.count + count <= SYS_INVGRIDMAXHOLD){
                    bin.count += count;
                    return;
                }

                count = bin.count + count - SYS_INVGRIDMAXHOLD;
                bin.count = SYS_INVGRIDMAXHOLD;;
            }
        }
    }

    // when reaching here
    // means we need a new grid for the item

    Pack2D pack2D(w());
    for(auto &bin: m_packBinList){
        pack2D.put(bin.x, bin.y, bin.w, bin.h);
    }

    for(size_t doneCount = 0; doneCount < count;){
        const auto currAdd = std::min<size_t>(ir.hasDBID() ? 1 : SYS_INVGRIDMAXHOLD, count - doneCount);
        auto addedBin = makePackBin(itemID, currAdd);
        pack2D.add(&addedBin, 1);
        m_packBinList.push_back(addedBin);
        doneCount += currAdd;
    }
}

void InvPack::add(uint32_t itemID, size_t count, int x, int y)
{
    if(!(itemID && count)){
        throw fflerror("invalid arguments: itemID = %llu, count = %zu", to_llu(itemID), count);
    }

    auto itemBin = makePackBin(itemID, count);
    if(!(x >= 0 && x + itemBin.w <= (int)(w()) && y >= 0)){
        add(itemID, count);
        return;
    }

    Pack2D pack2D(w());
    for(auto &bin: m_packBinList){
        pack2D.put(bin.x, bin.y, bin.w, bin.h);
    }

    if(pack2D.occupied(x, y, itemBin.w, itemBin.h, true)){
        add(itemID, count);
        return;
    }

    itemBin.x = x;
    itemBin.y = y;
    m_packBinList.push_back(itemBin);
}

size_t InvPack::remove(uint32_t itemID, size_t count, int x, int y)
{
    for(auto &bin: m_packBinList){
        if(true
                && bin.x  == x
                && bin.y  == y
                && bin.id == itemID){

            if(bin.count <= count){
                const auto doneCount = bin.count;
                std::swap(bin, m_packBinList.back());
                m_packBinList.pop_back();
                return doneCount;;
            }
            else{
                bin.count -= count;
                return count;
            }
        }
    }
    return 0;
}

PackBin InvPack::makePackBin(uint32_t itemID, size_t count)
{
    if(auto texPtr = g_itemDB->Retrieve(DBCOM_ITEMRECORD(itemID).pkgGfxID | 0X01000000)){
        const auto [itemPW, itemPH] = SDLDeviceHelper::getTextureSize(texPtr);
        return
        {
            .id    = itemID,
            .count = count,

            .x = -1,
            .y = -1,
            .w = (itemPW + (SYS_INVGRIDPW - 1)) / SYS_INVGRIDPW,
            .h = (itemPH + (SYS_INVGRIDPH - 1)) / SYS_INVGRIDPH,
        };
    }
    return {};
}
