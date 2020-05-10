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
int Pack2D::Occupied(int nX, int nY)
{
    if((nX >= 0) && (nX < (int)(W()))){

        // 1. box is outside current memory map
        //    always return not occupied
        if(nY >= (int)(m_packMap.size())){
            return 0;
        }

        // 2. box is inside current memory map
        //    need to check memory bit
        if(nY >= 0){
            return (m_packMap[nY] & (1 << nX)) ? 1 : 0;
        }
    }
    return -1;
}

int Pack2D::Occupied(int nX, int nY, int nW, int nH, bool bAny)
{
    if(true
            && nX >= 0
            && nY >= 0
            && nW >  0
            && nH >  0){

        for(int nCurrX = nX; nCurrX < nX + nW; ++nCurrX){
            for(int nCurrY = nY; nCurrY < nY + nH; ++nCurrY){
                switch(Occupied(nCurrX, nCurrY)){
                    case 0:
                        {
                            if(!bAny){
                                return 0;
                            }
                            break;
                        }
                    case 1:
                        {
                            if(bAny){
                                return 1;
                            }
                            break;
                        }
                    default:
                        {
                            return -1;
                        }
                }
            }
        }
        return bAny? 0 : 1;
    }
    return -1;
}

int Pack2D::Occupy(int nX, int nY, bool bOccup)
{
    if(true
            && nX >= 0 && nX < (int)(W())
            && nY >= 0){

        if((nY >= (int)(m_packMap.size()))){
            m_packMap.resize(nY + 1);
        }

        if(bOccup){
            m_packMap[nY] |= (1 << nX);
        }else{
            m_packMap[nY] |= (1 << nX);
            m_packMap[nY] ^= (1 << nX);
        }

        Shrink();
        return bOccup ? 1 : 0;
    }
    return -1;
}

int Pack2D::Occupy(int nX, int nY, int nW, int nH, bool bOccup)
{
    if(true
            && (nX >= 0)
            && (nY >= 0)
            && (nW >= 0)
            && (nH >= 0)

            && ((nX + nW) <= (int)(W()))){

        for(int nCurrX = nX; nCurrX < nX + nW; ++nCurrX){
            for(int nCurrY = nY; nCurrY < nY + nH; ++nCurrY){
                switch(Occupy(nCurrX, nCurrY, bOccup)){
                    case 0:
                    case 1:
                        {
                            break;
                        }
                    default:
                        {
                            return -1;
                        }
                }
            }
        }
        return bOccup ? 1 : 0;
    }
    return -1;
}

int Pack2D::FindRoom(PackBin *pBin)
{
    if(true
            && pBin
            && pBin->W >= 0
            && pBin->H >= 0){

        if(pBin->W <= (int)(W())){
            for(int nCurrY = 0; nCurrY <= (int)(m_packMap.size()); ++nCurrY){
                for(int nCurrX = 0; nCurrX <= (int)(W()) - pBin->W; ++nCurrX){
                    switch(Occupied(nCurrX, nCurrY, pBin->W, pBin->H, true)){
                        case 0:
                            {
                                pBin->X = nCurrX;
                                pBin->Y = nCurrY;
                                return 1;
                            }
                        case 1:
                            {
                                break;
                            }
                        default:
                            {
                                return -1;
                            }
                    }
                }
            }

            // shouldn't be here, since if the size fix
            // we should always put it inside successfully
            return 1;
        }
        return 0;
    }
    return -1;
}

int Pack2D::Pack(std::vector<PackBin> *pBinList)
{
    m_packMap.clear();
    if(pBinList){
        if(pBinList->empty()){
            return 1;
        }else{
            return Add(&((*pBinList)[0]), pBinList->size());
        }
    }
    return -1;
}

int Pack2D::Put(int nX, int nY, int nW, int nH)
{
    switch(Occupied(nX, nY, nW, nH, true)){
        case 0:
            {
                return Occupy(nX, nY, nW, nH, true);
            }
        case 1:
            {
                return 0;
            }
        default:
            {
                return -1;
            }
    }
}

int Pack2D::Add(PackBin *pBinList, size_t nBinCnt)
{
    if(pBinList && nBinCnt){
        auto fnBinCmp = [](const PackBin &rstLHS, const PackBin &rstRHS) -> bool
        {
            return rstLHS.ID < rstRHS.ID;
        };
        std::sort(pBinList, pBinList + nBinCnt, fnBinCmp);

        for(size_t nIndex = 0; nIndex < nBinCnt; ++nIndex){
            switch(auto nRet = FindRoom(pBinList + nIndex)){
                case 1:
                    {
                        // find room only 
                        // we need to explicitly take this room here
                        switch(Occupy(pBinList[nIndex].X, pBinList[nIndex].Y, pBinList[nIndex].W, pBinList[nIndex].H, true)){
                            case 1:
                                {
                                    break;
                                }
                            default:
                                {
                                    return -1;
                                }
                        }
                        break;
                    }
                case 0:
                default:
                    {
                        return nRet;
                    }
            }
        }
        return 1;
    }
    return -1;
}

int Pack2D::Remove(const PackBin &rstBin)
{
    if(Occupied(rstBin.X, rstBin.Y, rstBin.W, rstBin.H, false)){
        return Occupy(rstBin.X, rstBin.Y, rstBin.W, rstBin.H, false);
    }else{
        return -1;
    }
}
