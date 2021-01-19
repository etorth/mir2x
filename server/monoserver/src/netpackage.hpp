/*
 * =====================================================================================
 *
 *       Filename: netpackage.hpp
 *        Created: 05/03/2016 13:19:07
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
#include <cstdint>
#include <type_traits>

struct NetPackage // keep this POD
{
    uint8_t  type;
    uint8_t *dbuf;
    uint8_t  sbuf[256];
    size_t   size;

    const uint8_t *buf() const
    {
        return dbuf ? dbuf : sbuf;
    }
};
static_assert(std::is_trivially_copyable_v<NetPackage>);

inline void buildNetPackage(NetPackage *pkg, uint8_t type, const void *data, size_t dataLen)
{
    std::memset(pkg, 0, sizeof(NetPackage));
    pkg->type = type;
    pkg->size = dataLen;
    if(dataLen <= sizeof(pkg->sbuf)){
        std::memcpy(pkg->sbuf, data, dataLen);
    }
    else{
        pkg->dbuf = new uint8_t[dataLen];
        std::memcpy(pkg->dbuf, data, dataLen);
    }
}

template<typename T> void buildNetPackage(NetPackage *pkg, uint8_t type, const T &t)
{
    buildNetPackage(pkg, type, &t, sizeof(t));
}

inline void freeNetPackage(NetPackage *pkg)
{
    delete [] pkg->dbuf;
}
