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
    if(!(itemID && count)){
        throw fflerror("invalid arguments: itemID = %llu, count = %zu", to_llu(itemID), count);
    }

    if(!DBCOM_ITEMRECORD(itemID).hasDBID()){
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

    auto addedBin = makePackBin(itemID, count);
    pack2D.add(&addedBin, 1);
    m_packBinList.push_back(addedBin);
}

size_t InvPack::remove(uint32_t itemID, size_t count, int x, int y)
{
    for(auto &bin: m_packBinList){
        if(true
                && bin.x  == x
                && bin.y  == y
                && bin.id == itemID){

            if(bin.count <= count){
                std::swap(bin, m_packBinList.back());
                m_packBinList.pop_back();
                return bin.count;
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
