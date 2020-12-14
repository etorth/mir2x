/*
 * =====================================================================================
 *
 *       Filename: zcompf.hpp
 *        Created: 04/06/2020 10:50:49
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

#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <cstring>
#include <libpopcnt.h>
#include <type_traits>
#include "lz4.h"
#include "totype.hpp"
#include "fflerror.hpp"

namespace zcompf
{
    template<typename T> void lz4Encode(std::vector<uint8_t> &dst, const T *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable<T>::value);
        if(!(src && srcSize)){
            throw fflerror("invalid argument: src = %p, srcSize = %zu", src, srcSize);
        }

        const int maxDstSize = LZ4_compressBound(srcSize * sizeof(T));

        dst.clear();
        dst.resize(maxDstSize);

        const int compSize = LZ4_compress_default(reinterpret_cast<const char *>(src.data()), reinterpret_cast<char *>(dst.data()), srcSize * sizeof(T), maxDstSize);
        if(compSize <= 0){
            throw fflerror("LZ4_compress_default() return error code: %d", compSize);
        }
        dst.resize(compSize);
    }

    template<typename T> void lz4Encode(std::vector<uint8_t> &dst, const std::vector<T> &src)
    {
        lz4Encode<T>(dst, src.data(), src.size());
    }

    // lz4 has no reverse function of LZ4_compressBound()
    // the decompression bound is 255 * srcLen, this is too large, so need to provide the maxDstSize 

    template<typename T> void lz4Decode(std::vector<T> &dst, size_t maxDstSize, const uint8_t *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable<T>::value);
        if(!(maxDstSize && src && srcSize)){
            throw fflerror("invalid argument: maxDstSize = %zu, src = %p, srcSize = %zu", maxDstSize, src, srcSize);
        }

        dst.clear();
        dst.resize(maxDstSize);

        const int decompSize = LZ4_decompress_safe(reinterpret_cast<const char *>(src), reinterpret_cast<char *>(dst.data()), srcSize, maxDstSize * sizeof(T));
        if(decompSize <= 0){
            throw fflerror("LZ4_decompress_safe() return error code: %d", decompSize);
        }

        if(decompSize % sizeof(T)){
            throw fflerror("decompressed data is not aligned by object type T");
        }
        dst.resize(decompSize / sizeof(T));
    }

    template<typename T> void lz4Decode(std::vector<T> &dst, size_t maxDstSize, const std::vector<uint8_t> &src)
    {
        lz4Decode<T>(dst, maxDstSize, src.data(), src.size());
    }

    // xor compression/decompression
    // used by default, gives about 2x compression in general

    inline int countMask(const uint8_t *buf, size_t bufLen)
    {
        if(buf){
            return (int)(popcnt(buf, bufLen));
        }
        throw fflerror("invalid buffer: nullptr");
    }

    inline int countData(const uint8_t *buf, size_t bufLen)
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

    inline int xorEncode(uint8_t *dst, const uint8_t *buf, size_t bufLen)
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

    inline int xorDecode(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
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
}
