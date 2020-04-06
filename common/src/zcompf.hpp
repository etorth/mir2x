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
#include <type_traits>
#include "lz4.h"
#include "fflerror.hpp"

namespace zcompf
{
    template<typename T> void lz4Encode(std::vector<uint8_t> &dst, const T *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable<T>::value, "requires trivially copyable type");
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
        lz4Encode(dst, src.data(), src.size());
    }

    template<typename T> void lz4Decode(std::vector<T> &dst, size_t maxDstSize, const uint8_t *src, size_t srcSize)
    {
        static_assert(std::is_trivially_copyable<T>::value, "requires trivially copyable type");
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
        lzDecode(dst, maxDstSize, src.data(), src.size());
    }

    void xorEncode();
    void xorDecode();
}
