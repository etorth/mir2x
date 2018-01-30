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
#include "pngtexdbn.hpp"

bool InvPack::Repack()
{
    Pack2D stPack2D(W());
    switch(stPack2D.Pack(&m_PackBinList)){
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
        for(auto &rstPackBin: m_PackBinList){
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
                        m_PackBinList.push_back(stPackBin);
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
    for(size_t nIndex = 0; nIndex < m_PackBinList.size(); ++nIndex){
        if(true
                && m_PackBinList[nIndex].X  == nX
                && m_PackBinList[nIndex].Y  == nY
                && m_PackBinList[nIndex].ID == nItemID){
            std::swap(m_PackBinList[nIndex], m_PackBinList.back());
            m_PackBinList.pop_back();
            return true;
        }
    }
    return false;
}

PackBin InvPack::MakePackBin(uint32_t nItemID)
{
    extern PNGTexDBN *g_CommonItemDBN;
    if(auto pTexture = g_CommonItemDBN->Retrieve(nItemID - 1)){

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
