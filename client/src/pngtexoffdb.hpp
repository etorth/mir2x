/*
 * =====================================================================================
 *
 *       Filename: pngtexoffdb.hpp
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
#include <cstdint>
#include <SDL2/SDL.h>
#include <unordered_map>

#include "inndb.hpp"
#include "hexstring.hpp"
#include "sdldevice.hpp"

struct PNGTexOffItem
{
    SDL_Texture *Texture;
    int          DX;
    int          DY;
};

template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class PNGTexOffDB: public InnDB<uint32_t, PNGTexOffItem, LCDeepN, LCLenN, ResMaxN>
{
    private:
        struct ZIPItemInfo
        {
            zip_uint64_t Index;
            size_t       Size;

            // since DX/DY are in each filename
            // we store when loading
            // then each time we only load the graphics data
            int          DX;
            int          DY;
        };

    private:
        struct zip *m_ZIP;

    private:
        std::vector<uint8_t> m_Buf;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        PNGTexOffDB()
            : InnDB<uint32_t, PNGTexOffItem, LCDeepN, LCLenN, ResMaxN>()
            , m_ZIP(nullptr)
            , m_Buf()
            , m_ZIPItemInfoCache()
        {}

        virtual ~PNGTexOffDB()
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
                            //
                            // [0 ~ 7] [8] [9] [10 ~ 13] [14 ~ 17]
                            //  <KEY>  <S> <S>   <+DX>     <+DY>
                            //    4    1/2 1/2     2         2
                            //   
                            //   KEY: 3 bytes
                            //   S  : sign of DX, take 1 char, 1/2 byte, + for 1, - for 0
                            //   S  : sign of DY, take 1 char, 1/2 byte
                            //   +DX: abs(DX) take 4 chars, 2 bytes
                            //   +DY: abs(DY) take 4 chars, 2 bytes
                            //
                            // for key
                            uint32_t nKey = HexString::ToHex<uint32_t, 4>(stZIPStat.name);
                            // for DX, DY
                            int nDX, nDY;
                            nDX = (stZIPStat.name[8] != '0') ? 1 : (-1);
                            nDY = (stZIPStat.name[9] != '0') ? 1 : (-1);

                            nDX *= (int)HexString::ToHex<uint32_t, 2>(stZIPStat.name + 10);
                            nDY *= (int)HexString::ToHex<uint32_t, 2>(stZIPStat.name + 14);

                            m_ZIPItemInfoCache[nKey] = {stZIPStat.index, (size_t)stZIPStat.size, nDX, nDY};
                        }
                    }
                }
            }

            return Valid();
        }

    public:
        void RetrieveItem(uint32_t nKey, PNGTexOffItem *pItem, const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            if(pItem){
                // InnRetrieve always return true;
                this->InnRetrieve(nKey, pItem, fnLinearCacheKey, nullptr);
            }
        }

    public:
        // for all pure virtual function required in class InnDB;
        //
        virtual PNGTexOffItem LoadResource(uint32_t nKey)
        {
            // null resource desc
            PNGTexOffItem stItem {nullptr, 0, 0};

            auto pZIPIndexRecord = m_ZIPItemInfoCache.find(nKey);
            if(pZIPIndexRecord != m_ZIPItemInfoCache.end()){

                stItem.DX = pZIPIndexRecord->second.DX;
                stItem.DY = pZIPIndexRecord->second.DY;

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

        void FreeResource(PNGTexOffItem &stItem)
        {
            if(stItem.Texture){
                SDL_DestroyTexture(stItem.Texture);
            }
        }
};
