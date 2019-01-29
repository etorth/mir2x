/*
 * =====================================================================================
 *
 *       Filename: compress.cpp
 *        Created: 04/23/2017 21:34:23
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

#include <memory>
#include <cstring>
#include <libpopcnt.h>
#include "compress.hpp"

int Compress::CountMask(const uint8_t *pData, size_t nDataLen)
{
    if(!pData){
        return -1;
    }
    return (int)(popcnt(pData, nDataLen));
}

int Compress::CountData(const uint8_t *pData, size_t nDataLen)
{
    if(pData){
        int nCount = 0;
        for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
            nCount += (pData[nIndex] ? 1 : 0);
        }
        return nCount;
    }
    return -1;
}

int Compress::Encode(uint8_t *pDst, const uint8_t *pData, size_t nDataLen)
{
    if(pDst && pData && nDataLen){
        auto nMaskLen = ((nDataLen + 7) / 8);
        auto pMask = pDst;
        auto pComp = pDst + nMaskLen;

        std::memset(pMask, 0, nMaskLen);

        int nDataCount = 0;
        for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
            if(pData[nIndex]){
                pMask[nIndex / 8  ] |= (0X01 << (nIndex % 8));
                pComp[nDataCount++]  = pData[nIndex];
            }
        }
        return nDataCount;
    }
    return -1;
}

int Compress::Decode(uint8_t *pOrig, size_t nDataLen, const uint8_t *pMask, const uint8_t *pComp)
{
    if(pOrig && nDataLen && pMask && pComp){
        int nDecodeCount = 0;
        for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
            pOrig[nIndex] = (pMask[nIndex / 8] & (0x01 << (nIndex % 8))) ? pComp[nDecodeCount++] : 0;
        }
        return nDecodeCount;
    }
    return -1;
}
