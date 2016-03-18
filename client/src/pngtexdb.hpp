/*
 * =====================================================================================
 *
 *       Filename: pngtexdb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 03/17/2016 21:14:39
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
class PNGTexDB: public IntexDB<uint32_t, SDL_Texture *, DeepN, LenN, ResM>
{
    public:
        PNGTexDB() : IntexDB(){}
       ~PNGTexDB() = default;

    public:
        size_t   m_BufSize;
        uint8_t *m_Buf;

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

            return InnRetrieve(nKey, fnLinearCacheKey, fnZIPIndex, nullptr);
        }

        void ExtendBuf(size_t nSize)
        {
            if(nSize > m_BufSize){
                delete m_Buf;
                m_Buf = new uint8_t[nSize];
                m_BufSize = nSize;
            }
        }

    public:
        // for all pure virtual function required in class InresDB;
        SDL_Texture *LoadResource(uint32_t nKey)
        {
            auto pZIPIndexInst = m_ZIPIndexCache.find(nKey);
            if(pZIPIndexInst == m_ZIPIndexCache.end()){ return nullptr; }

            zip_stat_t stZIPStat;
            if(!zip_stat_index(m_ZIP, pZIPIndexInst->second, ZIP_FL_ENC_RAW, &stZIPStat)){
                // here we don't have to check agian actually
                if(true
                        && stZIPStat.valid & ZIP_STAT_INDEX
                        && stZIPStat.valid & ZIP_STAT_SIZE){

                    auto fp = zip_fopen_index(m_ZIP, stZIPStat.index, 0);
                    if(fp == nullptr){ return nullptr; }

                    ExtendBuf((size_t)stZIPStat.size);

                    if(stZIPStat.size != (zip_uint64_t)zip_fread(fp, m_Buf, stZIPStat.size)){
                        zip_fclose(fp);
                        zip_close(m_ZIP); m_ZIP = nullptr;
                        return nullptr;
                    }

                    extern SDLDevice *g_SDLDevice;
                    return g_SDLDevice->CreateTexture((const uint8_t *)m_Buf, (size_t)stZIPStat.size);
                }
            }
            return nullptr;
        }

        void FreeResource(SDL_Texture * &pTexture)
        {
            if(pTexture){
                SDL_DestroyTexture(pTexture);
            }
        }
};
