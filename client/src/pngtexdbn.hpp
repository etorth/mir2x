/*
 * =====================================================================================
 *
 *       Filename: pngtexdbn.hpp
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

template<size_t ResMaxN> class PNGTexDB: public InnDB<uint32_t, PNGTexItem, ResMaxN>
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
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        PNGTexDB()
            : InnDB<uint32_t, PNGTexItem, ResMaxN>()
            , m_ZIP(nullptr)
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

            if(!m_ZIP){
                return false;
            }

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
        SDL_Texture *Retrieve(uint32_t nKey)
        {
            if(PNGTexItem stItem {nullptr}; this->RetrieveResource(nKey, &stItem)){
                return stItem.Texture;
            }
            return nullptr;
        }

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage));
        }

    public:
        virtual std::tuple<PNGTexItem, size_t> LoadResource(uint32_t nKey)
        {
            PNGTexItem stItem {nullptr};
            if(auto pZIPIndexRecord = m_ZIPItemInfoCache.find(nKey); pZIPIndexRecord != m_ZIPItemInfoCache.end()){
                if(auto fp = zip_fopen_index(m_ZIP, pZIPIndexRecord->second.Index, ZIP_FL_UNCHANGED)){
                    std::vector<uint8_t> stBuf;
                    size_t nSize = pZIPIndexRecord->second.Size;

                    stBuf.resize(nSize);
                    if(nSize == (size_t)(zip_fread(fp, stBuf.data(), nSize))){
                        extern SDLDevice *g_SDLDevice;
                        stItem.Texture = g_SDLDevice->CreateTexture((const uint8_t *)(stBuf.data()), nSize);
                    }
                    zip_fclose(fp);
                }
            }
            return {stItem, stItem.Texture ? 1 : 0};
        }

        virtual void FreeResource(PNGTexItem &rstItem)
        {
            if(rstItem.Texture){
                SDL_DestroyTexture(rstItem.Texture);
                rstItem.Texture = nullptr;
            }
        }
};

using PNGTexDBN = PNGTexDB<1024>;
