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
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>
#include "zcompf.hpp"
#include "fflerror.hpp"

namespace cerealf
{
    template<typename T> std::string serialize(const T &t, bool compress)
    {
        std::ostringstream ss(std::ios::binary);
        cereal::BinaryOutputArchive ar(ss);

        ar(t);
        std::string rawBuf = ss.str();

        if(!compress){
            return rawBuf; // no NRVO, using move
        }

        std::string compBuf;
        zcompf::zstdEncode(compBuf, (const uint8_t *)(rawBuf.data()), rawBuf.size());
        return compBuf;
    }

    template<typename T> T deserialize(const void *buf, size_t size, bool decompress)
    {
        std::istringstream ss([buf, size, decompress]() -> std::string
        {
            if(!decompress){
                return std::string((const char *)(buf), size);
            }

            std::string decompBuf;
            zcompf::zstdDecode(decompBuf, (const uint8_t *)(buf), size);
            return decompBuf;
        }(), std::ios::binary);
        cereal::BinaryInputArchive ar(ss);

        T t;
        ar(t);
        return t;
    }

    template<typename T> T deserialize(std::string buf, bool decompress)
    {
        std::istringstream ss([&buf, decompress]() -> std::string
        {
            if(!decompress){
                return std::string(std::move(buf));
            }

            std::string decompBuf;
            zcompf::zstdDecode(decompBuf, (const uint8_t *)(buf.data()), buf.size());
            return decompBuf;
        }(), std::ios::binary);
        cereal::BinaryInputArchive ar(ss);

        T t;
        ar(t);
        return t;
    }
}
