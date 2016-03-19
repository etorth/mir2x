/*
 * =====================================================================================
 *
 *       Filename: pngtexdb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 03/18/2016 16:15:33
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
#include "inndb.hpp"
#include <unordered_map>
#include "hexstring.hpp"
#include <zip.h>

typedef struct{
    SDL_Texture *Texture;
}PNGTexItem;

template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class PNGTexDB: public InnDB<uint32_t, PNGTexItem, LCDeepN, LCLenN, ResMaxN>
{
    private:
        size_t   m_BufSize;
        uint8_t *m_Buf;
        zip_t   *m_ZIP;

    private:
        typedef struct{
            zip_uint64_t Index;
            size_t       Size;
        }ZIPItemInfo;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        PNGTexDB()
            : InnDB<uint32_t, PNGTexItem, LCDeepN, LCLenN, ResMaxN>()
            , m_BufSize(0)
            , m_Buf(nullptr)
            , m_ZIP(nullptr)
            , m_ZIPItemInfoCache()
        {}
        virtual ~PNGTexDB() = default;

    public:
        bool Valid()
        {
            return m_ZIP && !m_ZIPItemInfoCache.empty();
        }

        bool Load(const char *szPNGTexDBName)
        {
            int nErrorCode;
            m_ZIP = zip_open(szPNGTexDBName, ZIP_RDONLY, &nErrorCode);
            if(m_ZIP == nullptr){ return false; }

            zip_int64_t nCount = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
            if(nCount > 0){
                for(zip_uint64_t nIndex = 0; nIndex < (zip_uint64_t)nCount; ++nIndex){
                    zip_stat_t stZIPStat;
                    if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                        if(true
                                && stZIPStat.valid & ZIP_STAT_INDEX
                                && stZIPStat.valid & ZIP_STAT_SIZE
                                && stZIPStat.valid & ZIP_STAT_NAME){
                            uint32_t nKey = StringHex<uint32_t, 3>(stZIPStat.name);
                            m_ZIPItemInfoCache[nKey] = {stZIPStat.index, (size_t)stZIPStat.size};
                        }
                    }
                }
            }
            return Valid();
        }

    public:
        PNGTexItem RetrieveItem(uint32_t nKey,
                const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            PNGTexItem stItem {nullptr};

            // InnRetrieve always return true;
            this->InnRetrieve(nKey, &stItem, fnLinearCacheKey, nullptr);

            return stItem.Texture;
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
        // for all pure virtual function required in class InnDB;
        //
        virtual PNGTexItem LoadResource(uint32_t nKey)
        {
            auto pZIPIndexInst = m_ZIPItemInfoCache.find(nKey);
            if(pZIPIndexInst == m_ZIPItemInfoCache.end()){ return nullptr; }

            auto fp = zip_fopen_index(m_ZIP, pZIPIndexInst->second.Index, ZIP_FL_UNCHANGED);
            if(fp == nullptr){ return nullptr; }

            size_t nSize = pZIPIndexInst->second.Size;
            ExtendBuf(nSize);

            if(nSize != (size_t)zip_fread(fp, m_Buf, nSize)){
                zip_fclose(fp);
                return nullptr;
            }

            extern SDLDevice *g_SDLDevice;
            return {g_SDLDevice->CreateTexture((const uint8_t *)m_Buf, nSize)};
        }

        void FreeResource(PNGTexItem &rstItem)
        {
            if(rstItem.Texture){
                SDL_DestroyTexture(rstItem.Texture);
            }
        }
};
