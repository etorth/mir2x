/*
 * =====================================================================================
 *
 *       Filename: shadow.cpp
 *        Created: 06/22/2017 17:05:41
 *  Last Modified: 06/22/2017 17:44:11
 *
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

#include <new>
#include <cstdio>
#include <cstring>
#include "shadow.hpp"

uint32_t *Shadow::MakeShadow(uint32_t *pDst,
        bool bProject,
        const uint32_t *pData,
        int nW,
        int nH,
        int *pSW,
        int *pSH,
        uint32_t nShadowColor)
{
    if(false
            || pData == nullptr
            || nW <= 0
            || nH <= 0){ return nullptr; }

    // shadow size depends on project or not
    //
    //      project :  (nW + nH / 2) x (nH / 2 + 1)
    //              :  (nW x nH)
    //
    // allocate if not provided
    // if user provided pDst buffer, it must be of enough size

    int nNewW = bProject ? (nW + nH / 2) : nW;
    int nNewH = bProject ? ( 1 + nH / 2) : nH;

    if(pDst == nullptr){
        pDst = new(std::nothrow) uint32_t [nNewW * nNewH * sizeof(uint32_t)];
    }

    if(pDst == nullptr){
        std::printf("allocate memory failed : %d x %d x %d", nNewW, nNewH, (int)(sizeof(uint32_t)));
        return nullptr;
    }

    if(bProject){
        std::memset(pDst, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            int nYCnt = nY - nY / 2;
            for(int nX = 0; nX < nW; ++nX){
                int nXCnt = nX + (nH - nY) / 2;
                if(pData[nY * nW + nX] & 0XFF000000){
                    pDst[nYCnt * nNewW + nXCnt] = nShadowColor;
                }
            }
        }
    }else{
        std::memset(pDst, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            for(int nX = 0; nX < nW; ++nX){
                if(pData[nY * nW + nX] & 0XFF000000){
                    pDst[nY * nW + nX] = nShadowColor;
                }
            }
        }
    }

    if(pSW){ *pSW = nNewW; }
    if(pSH){ *pSH = nNewH; }

    return pDst;
}
