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
#include "totype.hpp"
#include "compress.hpp"
#include "fflerror.hpp"

int Compress::countMask(const uint8_t *buf, size_t bufLen)
{
    if(buf){
        return (int)(popcnt(buf, bufLen));
    }
    throw fflerror("invalid buffer: nullptr");
}

int Compress::countData(const uint8_t *buf, size_t bufLen)
{
    if(buf){
        int count = 0;
        for(size_t i = 0; i < bufLen; ++i){
            if(buf[i]){
                count++;
            }
        }
        return count;
    }
    throw fflerror("invalid buffer: nullptr");
}

int Compress::xorEncode(uint8_t *dst, const uint8_t *buf, size_t bufLen)
{
    if(dst && buf && bufLen){
        auto maskLen = ((bufLen + 7) / 8);
        auto maskPtr = dst;
        auto compPtr = dst + maskLen;
        std::memset(maskPtr, 0, maskLen);

        int dataCount = 0;
        for(size_t i = 0; i < bufLen; ++i){
            if(buf[i]){
                maskPtr[i / 8] |= (0X01 << (i % 8));
                compPtr[dataCount++] = buf[i];
            }
        }
        return dataCount;
    }
    throw fflerror("invalid arguments: dst = %p, buf = %p, bufPtr = %zu", to_cvptr(dst), to_cvptr(buf), bufLen);
}

int Compress::xorDecode(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
{
    if(dst && bufLen && maskPtr && compPtr){
        int decodeCount = 0;
        for(size_t i = 0; i < bufLen; ++i){
            if(maskPtr[i / 8] & (0x01 << (i % 8))){
                dst[i] = compPtr[decodeCount++];
            }
            else{
                dst[i] = 0;
            }
        }
        return decodeCount;
    }
    throw fflerror("invalid arguments: dst = %p, bufLen = %zu, maskPtr = %p, compPtr = %p", to_cvptr(dst), bufLen, to_cvptr(maskPtr), to_cvptr(compPtr));
}
