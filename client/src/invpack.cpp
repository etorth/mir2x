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

#include "invpack.hpp"
#include "pngtexdb.hpp"

extern PNGTexDB *g_commonItemDB;

bool InvPack::Repack()
{
    Pack2D stPack2D(W());
    switch(stPack2D.Pack(&m_packBinList)){
        case 1:
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}

bool InvPack::Add(uint32_t nItemID)
{
    if(nItemID){
        Pack2D stPack2D(W());
        for(auto &rstPackBin: m_packBinList){
            switch(stPack2D.Put(rstPackBin.X, rstPackBin.Y, rstPackBin.W, rstPackBin.H)){
                case 1:
                    {
                        break;
                    }
                default:
                    {
                        return false;
                    }
            }
        }

        if(auto stPackBin = MakePackBin(nItemID)){
            switch(stPack2D.Add(&stPackBin)){
                case 1:
                    {
                        m_packBinList.push_back(stPackBin);
                        return true;
                    }
                default:
                    {
                        return false;
                    }
            }
        }
    }
    return false;
}

bool InvPack::Remove(uint32_t nItemID, int nX, int nY)
{
    for(size_t nIndex = 0; nIndex < m_packBinList.size(); ++nIndex){
        if(true
                && m_packBinList[nIndex].X  == nX
                && m_packBinList[nIndex].Y  == nY
                && m_packBinList[nIndex].ID == nItemID){
            std::swap(m_packBinList[nIndex], m_packBinList.back());
            m_packBinList.pop_back();
            return true;
        }
    }
    return false;
}

PackBin InvPack::MakePackBin(uint32_t nItemID)
{
    if(auto pTexture = g_commonItemDB->Retrieve(nItemID - 1)){

        int nItemPW = -1;
        int nItemPH = -1;
        if(!SDL_QueryTexture(pTexture, nullptr, nullptr, &nItemPW, &nItemPH)){

            int nItemGW = (nItemPW + (SYS_INVGRIDPW - 1)) / SYS_INVGRIDPW;
            int nItemGH = (nItemPH + (SYS_INVGRIDPH - 1)) / SYS_INVGRIDPH;

            return {nItemID, -1, -1, nItemGW, nItemGH};
        }
    }
    return {0};
}
