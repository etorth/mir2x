/*
 * =====================================================================================
 *
 *       Filename: mapbindbn.hpp
 *        Created: 09/05/2017 10:33:14
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
#include "mapbindb.hpp"

#define MAPBINDBN_LC_DEPTH  0
#define MAPBINDBN_LC_LENGTH 0
#define MAPBINDBN_CAPACITY  2

using MapBinDBType = MapBinDB<MAPBINDBN_LC_DEPTH, MAPBINDBN_LC_LENGTH, MAPBINDBN_CAPACITY>;

class MapBinDBN: public MapBinDBType
{
    public:
        MapBinDBN()
            : MapBinDBType()
        {}

    public:
        virtual ~MapBinDBN() = default;

    public:
        Mir2xMapData *Retrieve(uint32_t nKey)
        {
            const auto &fnLinearCacheKey = [](uint32_t) -> size_t
            {
                return 0;
            };

            return RetrieveItem(nKey, fnLinearCacheKey).Map;
        }
};
