/*
 * =====================================================================================
 *
 *       Filename: compress.cpp
 *        Created: 04/23/2017 21:34:23
 *  Last Modified: 12/15/2017 19:31:43
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

#include <memory>
#include <cstring>
#include "compress.hpp"

int Compress::CountMask(const uint8_t *pData, size_t nDataLen)
{
    if(pData){
        int  nMaskCount = 0;
        auto pOrigBegin = pData;

        for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
            nMaskCount += __builtin_popcount(pData[nIndex]);
        }
        return nMaskCount;

        // need better understanding of std::align()
        // http://en.cppreference.com/w/cpp/memory/align
        // document says it does nothing if the given buffer is too small to fit

        // we may have smaller memory chunk at the begin and end of pData
        // but [pData, pData + nDataLen) is enough to hold at least one unsign long long if not return nullptr

        // nasty casts, std::align only accpet (void * &)
        auto pCastData = static_cast<void *>(const_cast<uint8_t *>(pData));

        // if succeed after this call pData and nDataLen will be adjusted
        if(std::align(alignof(unsigned long long), sizeof(unsigned long long), pCastData, nDataLen)){
            for(size_t nIndex = 0; nIndex < nDataLen; nIndex += sizeof(unsigned long long)){
                // TODO remove the strict-aliasing issue here
                nMaskCount += __builtin_popcountll(*((unsigned long long *)((const uint8_t *)(pCastData) + nIndex)));
            }
        }

        // we always do it no matter align succeed or not
        pData = (const uint8_t *)(pCastData);

        // for (possible) memory chunk at the begin
        // pData could be adjusted by std::align() if succeed
        for(const uint8_t *pBegin = pOrigBegin; pBegin < pData; ++pBegin){
            nMaskCount += __builtin_popcount((unsigned int)(*pBegin));
        }

        // for (possible) memory chunk at the end
        // pData and nDataLen could be adjusted by std::align() if succeed
        if(auto nRest = nDataLen % sizeof(unsigned long long)){
            for(size_t nIndex = nDataLen - nRest; nIndex < nDataLen; ++nIndex){
                nMaskCount += __builtin_popcount((unsigned int)(pData[nIndex]));
            }
        }

        // done, return the count
        return nMaskCount;
    }
    return -1;
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
