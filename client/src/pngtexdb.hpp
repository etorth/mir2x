/*
 * =====================================================================================
 *
 *       Filename: pngtexdb.hpp
 *        Created: 02/26/2016 21:48:43
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
#include <zip.h>
#include <vector>
#include <unordered_map>

#include "inndb.hpp"
#include "hexstring.hpp"
#include "sdldevice.hpp"

struct PNGTexItem
{
    SDL_Texture *Texture;
};

template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class PNGTexDB: public InnDB<uint32_t, PNGTexItem, LCDeepN, LCLenN, ResMaxN>
{
    private:
        struct ZIPItemInfo
        {
            zip_uint64_t Index;
            size_t       Size;
        };

    private:
        struct zip *m_ZIP;

    private:
        std::vector<uint8_t> m_Buf;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        PNGTexDB()
            : InnDB<uint32_t, PNGTexItem, LCDeepN, LCLenN, ResMaxN>()
            , m_ZIP(nullptr)
            , m_Buf()
            , m_ZIPItemInfoCache()
        {}

        virtual ~PNGTexDB()
        {
            if(m_ZIP){
                zip_close(m_ZIP);
            }
        }

    public:
        bool Valid()
        {
            return m_ZIP && !m_ZIPItemInfoCache.empty();
        }

        bool Load(const char *szPNGTexDBName)
        {
            int nErrorCode = 0;

#ifdef ZIP_RDONLY
            m_ZIP = zip_open(szPNGTexDBName, ZIP_CHECKCONS | ZIP_RDONLY, &nErrorCode);
#else
            m_ZIP = zip_open(szPNGTexDBName, ZIP_CHECKCONS, &nErrorCode);
#endif

            if(!m_ZIP){ return false; }

            if(nErrorCode){
                if(m_ZIP){
                    zip_close(m_ZIP);
                    m_ZIP = nullptr;
                }
                return false;
            }

            zip_int64_t nCount = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
            if(nCount > 0){
                for(zip_uint64_t nIndex = 0; nIndex < (zip_uint64_t)(nCount); ++nIndex){
                    struct zip_stat stZIPStat;
                    if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                        if(true
                                && stZIPStat.valid & ZIP_STAT_INDEX
                                && stZIPStat.valid & ZIP_STAT_SIZE
                                && stZIPStat.valid & ZIP_STAT_NAME){
                            uint32_t nKey = HexString::ToHex<uint32_t, 4>(stZIPStat.name);
                            m_ZIPItemInfoCache[nKey] = {stZIPStat.index, (size_t)(stZIPStat.size)};
                        }
                    }
                }
            }

            return Valid();
        }

    public:
        PNGTexItem RetrieveItem(uint32_t nKey, const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            PNGTexItem stItem {nullptr};

            // InnRetrieve always return true;
            this->InnRetrieve(nKey, &stItem, fnLinearCacheKey, nullptr);
            return stItem;
        }

    public:
        // for all pure virtual function required in class InnDB;
        //
        virtual PNGTexItem LoadResource(uint32_t nKey)
        {
            PNGTexItem stItem {nullptr};

            auto pZIPIndexRecord = m_ZIPItemInfoCache.find(nKey);
            if(pZIPIndexRecord != m_ZIPItemInfoCache.end()){
                if(auto fp = zip_fopen_index(m_ZIP, pZIPIndexRecord->second.Index, ZIP_FL_UNCHANGED)){
                    size_t nSize = pZIPIndexRecord->second.Size;
                    m_Buf.resize(nSize);

                    if(nSize == (size_t)(zip_fread(fp, &(m_Buf[0]), nSize))){
                        extern SDLDevice *g_SDLDevice;
                        stItem.Texture = g_SDLDevice->CreateTexture((const uint8_t *)(&(m_Buf[0])), nSize);
                    }

                    zip_fclose(fp);
                }
            }

            return stItem;
        }

        void FreeResource(PNGTexItem &rstItem)
        {
            if(rstItem.Texture){
                SDL_DestroyTexture(rstItem.Texture);
            }
        }
};
