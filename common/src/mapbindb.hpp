/*
 * =====================================================================================
 *
 *       Filename: mapbindb.hpp
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
#include "hexstr.hpp"
#include "dbcomrecord.hpp"
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
            MapBinEntry stEntry {nullptr};
            if(const auto utf8name = DBCOM_MAPRECORD(nKey).name; str_haschar(utf8name)){
                const auto mapName = std::string(to_cstr(utf8name));
                if(const auto pos = mapName.find('_'); pos != std::string::npos){
                    if(const auto fileName = mapName.substr(pos + 1); !fileName.empty()){
                        if(std::vector<uint8_t> stBuf; m_ZSDBPtr->decomp(str_printf("%s.bin", fileName.c_str()).c_str(), 0, &stBuf)){
                            auto pMap = new Mir2xMapData();
                            pMap->loadData(stBuf.data(), stBuf.size());
                            stEntry.Map = pMap;
                        }
                    }
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
