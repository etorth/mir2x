/*
 * =====================================================================================
 *
 *       Filename: pngtexdb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 03/17/2016 19:11:47
 *
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
#include "intexdb.hpp"
#include <unordered_map>

template<size_t DeepN, size_t LenN, size_t ResM>
class PNGTexDB: public IntexDB<uint32_t, DeepN, LenN, ResM>
{
    public:
        PNGTexDB() : IntexDB(){}
       ~PNGTexDB() = default;

    public:
        bool Valid()
        {
            return ValidZIP() && !m_ZIPIndexCache.empty();
        }

        bool Load(const char *szPNGTexDBName)
        {
            if(LoadZIP(szPNGTexDBName)){
                zip_int64_t nCount = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
                if(nCount > 0){
                    for(zip_uint64_t nIndex = 0; nIndex < (zip_uint64_t)nCount; ++nIndex){
                        zip_stat_t stZIPStat;
                        if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                            if(true
                                    && stZIPStat.valid & ZIP_STAT_INDEX
                                    && stZIPStat.valid & ZIP_STAT_SIZE
                                    && stZIPStat.valid & ZIP_STAT_NAME){
                                uint32_t nKey = PNGNameKey(stZIPStat.name, 6);
                                m_ZIPIndexCache[nKey] = stZIPStat.index;
                            }
                        }
                    }
                }
            }
            return !m_ZIPIndexCache.empty();
        }

        void Clear()
        {
            ClearLUT();
            m_ZIPIndexCache.clear();
        }

    public:

        static size_t LinearCacheKey(uint32_t nKey)
        {
        }

        static zip_int64_t ZIPIndex()

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage));
        }

        SDL_Texture *Retrieve(uint32_t nKey)
        {
            const auto &fnLinearCacheKey = [&](uint32_t nKey)
            {
                return (nKey & 0X0000FFFF) % LenN;
            };

            const auto &fnZIPIndex = [&](uint32_t nKey)
            {
                return m_ZIPIndexCache[nKey];
            }

            size_t nLCBucketIndex;
            return InnRetrieve(nKey, fnLinearCacheKey, fnZIPIndex, &nLCBucketIndex);
        }
};
