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
#include <memory>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "dbcomrecord.hpp"
#include "mir2xmapdata.hpp"

struct MapBinElement
{
    std::shared_ptr<Mir2xMapData> data {};
};

class MapBinDB: public innDB<uint32_t, MapBinElement, true> // threadSafe
{
    private:
        std::unique_ptr<ZSDB> m_ZSDBPtr;

    public:
        MapBinDB()
            : innDB<uint32_t, MapBinElement, true>(16)
        {}

    public:
        bool load(const char *mapDBName)
        {
            m_ZSDBPtr = std::make_unique<ZSDB>(mapDBName);
            return true;
        }

    public:
        std::shared_ptr<Mir2xMapData> retrieve(uint32_t key)
        {
            if(auto p = innLoad(key)){
                return p->data; // return a shared copy which can sustain
            }
            return nullptr;
        }

    public:
        std::optional<std::tuple<MapBinElement, size_t>> loadResource(uint32_t key) override
        {
            if(const auto utf8name = DBCOM_MAPRECORD(key).name; str_haschar(utf8name)){
                const auto mapName = std::string(to_cstr(utf8name));
                if(const auto pos = mapName.find('_'); pos != std::string::npos){
                    if(const auto fileName = mapName.substr(pos + 1); !fileName.empty()){
                        if(std::vector<uint8_t> dataBuf; m_ZSDBPtr->decomp(str_printf("%s.bin", fileName.c_str()).c_str(), 0, &dataBuf)){
                            return std::make_tuple(MapBinElement
                            {
                                .data = std::make_shared<Mir2xMapData>(dataBuf.data(), dataBuf.size()),
                            }, 1);
                        }
                    }
                }
            }
            return {};
        }

        virtual void freeResource(MapBinElement &element)
        {
            element.data.reset(); // this only decreases ref count by 1
        }
};
