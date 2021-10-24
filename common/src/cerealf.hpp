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
        CF_NONE     = 0,
        CF_COMPRESS = 1,
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
        if((tryComp < 0 && rawBuf.size() < 32) || tryComp == 0){
            rawBuf.push_back(CF_NONE);
            return rawBuf;
        }

        std::string compBuf;
        zcompf::zstdEncode(compBuf, (const uint8_t *)(rawBuf.data()), rawBuf.size());

        if(compBuf.size() >= rawBuf.size()){
            rawBuf.push_back(CF_NONE);
            return rawBuf;
        }

        compBuf.push_back(CF_COMPRESS);
        return compBuf;
    }

    template<typename T> T deserialize(const void *buf, size_t size)
    {
        if(!(buf && (size >= 1))){
            throw fflerror("invalid arguments: buf = %p, size = %zu", buf, size);
        }

        const auto flag = ((char *)(buf))[--size];
        const bool decompress = flag & CF_COMPRESS;
        std::istringstream ss([buf, size, decompress]() -> std::string
        {
            if(!decompress){
                return std::string((const char *)(buf), size);
            }

            std::string decompBuf;
            zcompf::zstdDecode(decompBuf, (const uint8_t *)(buf), size);
            return decompBuf;
        }(), std::ios::binary);

        T t;
        {
            cereal::PortableBinaryInputArchive ar(ss);
            ar(t);
        }
        return t;
    }

    template<typename T> T deserialize(std::string buf)
    {
        if(buf.size() < 1){
            throw fflerror("invalid buf size: %zu", buf.size());
        }

        const auto flag = buf.back();
        const bool decompress = flag & CF_COMPRESS;

        buf.pop_back();
        std::istringstream ss([&buf, decompress]() -> std::string
        {
            if(!decompress){
                return std::string(std::move(buf));
            }

            std::string decompBuf;
            zcompf::zstdDecode(decompBuf, (const uint8_t *)(buf.data()), buf.size());
            return decompBuf;
        }(), std::ios::binary);

        T t;
        {
            cereal::PortableBinaryInputArchive ar(ss);
            ar(t);
        }
        return t;
    }
}
