/*
 * =====================================================================================
 *
 *       Filename: cerealf.hpp
 *        Created: 11/13/2018 22:31:02
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
#include <string>
#include <sstream>
#include <climits>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include "zcompf.hpp"
#include "fflerror.hpp"

namespace cerealf
{
    enum CerealfFlagType: char
    {
        CF_NONE = 0,
        CF_ZSTD = 1,
        CF_XOR  = 2,
    };

    // tryComp : -1 : auto
    //            0 : don't try compress
    //          >=1 :       try compress
    template<typename T> std::string serialize(const T &t, int tryComp = -1)
    {
        std::ostringstream ss(std::ios::binary);
        {
            cereal::PortableBinaryOutputArchive ar(ss);
            ar(t);
        }

        std::string rawBuf = ss.str();
        if((tryComp < 0 && rawBuf.size() < 8) || tryComp == 0){
            rawBuf.push_back(CF_NONE);
            return rawBuf;
        }

        std::string compBuf;
        if(rawBuf.size() < std::min<size_t>(64, CHAR_MAX)){
            const size_t maskSize = (rawBuf.size() + 7) / 8;
            compBuf.resize(maskSize + rawBuf.size() + 2);
            const size_t dataCount = zcompf::xorEncode((uint8_t *)(compBuf.data()), (const uint8_t *)(rawBuf.data()), rawBuf.size());
            const size_t totalSize = maskSize + dataCount;

            if(totalSize + 2 < rawBuf.size()){
                compBuf.resize(totalSize);
                compBuf.push_back(static_cast<char>(rawBuf.size())); // make sure rawBuf.size() < CHAR_MAX
                compBuf.push_back(CF_XOR);
                return compBuf;
            }
        }

        compBuf.clear(); // optional
        zcompf::zstdEncode(compBuf, (const uint8_t *)(rawBuf.data()), rawBuf.size());

        if(compBuf.size() + 1 < rawBuf.size()){
            compBuf.push_back(CF_ZSTD);
            return compBuf;
        }

        rawBuf.push_back(CF_NONE);
        return rawBuf;
    }

    template<typename T> T deserialize(const void *buf, size_t size)
    {
        fflassert(buf);
        fflassert(size >= 1);

        const auto flag = ((const char *)(buf))[--size];
        std::istringstream ss([buf, &size, flag]() -> std::string
        {
            switch(flag){
                case CF_NONE:
                    {
                        return std::string((const char *)(buf), size);
                    }
                case CF_XOR:
                    {
                        fflassert(size >= 1);
                        const auto bufLen = ((const uint8_t *)(buf))[--size];
                        const size_t maskLen = (bufLen + 7) / 8;

                        std::string decompBuf;
                        decompBuf.resize(bufLen);

                        zcompf::xorDecode((uint8_t *)(decompBuf.data()), bufLen, (const uint8_t *)(buf), (const uint8_t *)(buf) + maskLen);
                        return decompBuf;
                    }
                case CF_ZSTD:
                    {
                        std::string decompBuf;
                        zcompf::zstdDecode(decompBuf, (const uint8_t *)(buf), size);
                        return decompBuf;
                    }
                default:
                    {
                        throw bad_value(flag);
                    }
            }
        }(), std::ios::binary);

        T t;
        {
            cereal::PortableBinaryInputArchive ar(ss);
            ar(t);
        }
        return t;
    }

    template<typename T> T deserialize(const std::string &buf)
    {
        return deserialize<T>(buf.data(), buf.size());
    }
}
