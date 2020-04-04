/*
 * =====================================================================================
 *
 *       Filename: mapbindbn.hpp
 *        Created: 08/31/2017 17:23:35
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
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstring.hpp"
#include "mir2xmapdata.hpp"

struct MapBinEntry
{
    Mir2xMapData *Map;
};

class MapBinDB: public innDB<uint32_t, MapBinEntry>
{
    private:
        std::unique_ptr<ZSDB> m_ZSDBPtr;

    public:
        MapBinDB()
            : innDB<uint32_t, MapBinEntry>(16)
            , m_ZSDBPtr()
        {}

    public:
        bool Load(const char *szMapDBName)
        {
            try{
                m_ZSDBPtr = std::make_unique<ZSDB>(szMapDBName);
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        Mir2xMapData *Retrieve(uint32_t nKey)
        {
            if(MapBinEntry stEntry {nullptr}; this->RetrieveResource(nKey, &stEntry)){
                return stEntry.Map;
            }
            return nullptr;
        }

    public:
        virtual std::tuple<MapBinEntry, size_t> loadResource(uint32_t nKey)
        {
            char szKeyString[16];
            MapBinEntry stEntry {nullptr};

            if(std::vector<uint8_t> stBuf; m_ZSDBPtr->Decomp(HexString::ToString<uint32_t, 4>(nKey, szKeyString, true), 8, &stBuf)){
                auto pMap = new Mir2xMapData();
                if(pMap->Load(stBuf.data(), stBuf.size())){
                    stEntry.Map = pMap;
                }else{
                    delete pMap;
                }
            }
            return {stEntry, stEntry.Map ? 1 : 0};
        }

        virtual void freeResource(MapBinEntry &rstEntry)
        {
            if(rstEntry.Map){
                delete rstEntry.Map;
                rstEntry.Map = nullptr;
            }
        }
};
