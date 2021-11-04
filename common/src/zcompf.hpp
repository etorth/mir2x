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
#include <string>
#include <memory>
#include <cstring>
#include <libpopcnt.h>
#include <type_traits>
#include "lz4.h"
#include "zstd.h"
#include "fflerror.hpp"

namespace zcompf
{
    template<typename T, typename C = std::string> void lz4Encode(C &dst, const T *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable_v<T>);
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

    // lz4 has no reverse function of LZ4_compressBound()
    // the decompression bound is 255 * srcLen, this is too large, so need to provide the maxDstSize

    template<typename C> void lz4Decode(C &dst, size_t maxDstSize, const uint8_t *src, size_t srcSize)
    {
        using VALUE_TYPE = typename C::value_type;
        static_assert(std::is_trivially_copyable_v<VALUE_TYPE>);

        if(!(maxDstSize && src && srcSize)){
            throw fflerror("invalid argument: maxDstSize = %zu, src = %p, srcSize = %zu", maxDstSize, src, srcSize);
        }

        dst.clear();
        dst.resize(maxDstSize);

        const int decompSize = LZ4_decompress_safe(reinterpret_cast<const char *>(src), reinterpret_cast<char *>(dst.data()), srcSize, maxDstSize * sizeof(VALUE_TYPE));
        if(decompSize <= 0){
            throw fflerror("LZ4_decompress_safe() return error code: %d", decompSize);
        }

        if(decompSize % sizeof(VALUE_TYPE)){
            throw fflerror("decompressed data buffer is not aligned by class value type: size = %zu, sizeof(value_type) = %zu", decompSize, sizeof(VALUE_TYPE));
        }
        dst.resize(decompSize / sizeof(VALUE_TYPE));
    }

    // zstd compression/decompression
    template<typename T, typename C = std::string> void zstdEncode(C &dst, const T *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        if(!(src && srcSize)){
            throw fflerror("invalid argument: src = %p, srcSize = %zu", src, srcSize);
        }

        dst.clear();
        dst.resize(ZSTD_compressBound(srcSize * sizeof(T)));

        const size_t rc = ZSTD_compress(dst.data(), dst.size(), src, srcSize * sizeof(T), ZSTD_maxCLevel());
        if(ZSTD_isError(rc)){
            throw fflerror("failed to compress file: %s", ZSTD_getErrorName(rc));
        }
        dst.resize(rc);
    }

    template<typename C> void zstdDecode(C &dst, const void *src, size_t srcSize)
    {
        using VALUE_TYPE = typename C::value_type;
        static_assert(std::is_trivially_copyable_v<VALUE_TYPE>);

        switch(const auto decompSize = ZSTD_getFrameContentSize(src, srcSize)){
            case ZSTD_CONTENTSIZE_ERROR:
            case ZSTD_CONTENTSIZE_UNKNOWN:
                {
                    throw fflerror("not a zstd compressed data buffer: src = %p, srcSize = %zu", src, srcSize);
                }
            default:
                {
                    dst.resize(decompSize / sizeof(VALUE_TYPE) + 1);
                    break;
                }
        }

        const size_t rc = ZSTD_decompress(dst.data(), dst.size() * sizeof(VALUE_TYPE), src, srcSize);
        if(ZSTD_isError(rc)){
            throw fflerror("failed to decompress data buffer: %s", ZSTD_getErrorName(rc));
        }

        if(rc % sizeof(VALUE_TYPE)){
            throw fflerror("decompressed data buffer is not aligned by class value type: size = %zu, sizeof(value_type) = %zu", rc, sizeof(VALUE_TYPE));
        }
        dst.resize(rc / sizeof(VALUE_TYPE));
    }

    // xor compression/decompression
    // used by default, gives about 2x compression in general

    inline size_t countMask(const uint8_t *buf, size_t bufLen)
    {
        if(bufLen > 0){
            fflassert(buf);
            return popcnt(buf, bufLen);
        }
        return 0;
    }

    inline size_t countData(const uint8_t *buf, size_t bufLen)
    {
        size_t count = 0;
        for(size_t i = 0; i < bufLen; ++i){
            if(buf[i]){
                count++;
            }
        }
        return count;
    }

    inline size_t xorEncode(uint8_t *dst, const uint8_t *buf, size_t bufLen)
    {
        fflassert(dst);
        fflassert(buf);
        fflassert(bufLen);

        const auto maskLen = (bufLen + 7) / 8;
        const auto maskPtr = dst;
        const auto compPtr = dst + maskLen;
        std::memset(maskPtr, 0, maskLen);

        size_t dataCount = 0;
        for(size_t i = 0; i < bufLen; ++i){
            if(buf[i]){
                maskPtr[i / 8] |= (0X01 << (i % 8));
                compPtr[dataCount++] = buf[i];
            }
        }
        return dataCount;
    }

    inline size_t xorDecode(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
    {
        fflassert(dst);
        fflassert(bufLen);

        fflassert(maskPtr);
        fflassert(compPtr);

        size_t decodeCount = 0;
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
}
