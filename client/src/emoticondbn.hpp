/*
 * =====================================================================================
 *
 *       Filename: emoticondbn.hpp
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
#include <SDL2/SDL.h>
#include <unordered_map>

#include "inndb.hpp"
#include "hexstring.hpp"
#include "sdldevice.hpp"

// layout of a emoticon on a texture
// on a single picture, from left to right
// |
// V  ->| FW |<-
// ---- +----+----+----+----+-+
//      |    |    |    |    | |
// FH   | 0  | 1  | 2  | 3  | |
//      |    |    |    |    | |
// ---- +----+----+----+----+-+
// ^    |    |    |    |    | |
// |    | 4  | 5  | 6  | 7  | |
//      |    |    |    |    | |
//      +----+----+----+----+-+
//      |    |    |    |    | |
//      +----+----+----+----+-+
//
//  FileName format:
//
//  [0 ~ 1] [2 ~ 5] [6 ~ 7] [8 ~ 9] [10 ~ 13] [14 ~ 17] [18 ~ 21].PNG
//
//  ESet    ESubset EFrame  FPS     FrameW    FrameH    FrameH1
//  1Byte   2Bytes  1Byte   1Byte   2Bytes    2Bytes    2Byte
//                  ------
//                    ^
//                    |
//                    +------- always set it as zero
//                             and when retrieving we can just plus the frame index here

struct EmoticonItem
{
    SDL_Texture *Texture;
    int          FrameW;
    int          FrameH;
    int          FrameH1;
    int          FPS;
};

template<size_t ResMaxN> class EmoticonDB: public InnDB<uint32_t, EmoticonItem, ResMaxN>
{
    private:
        struct ZIPItemInfo
        {
            zip_uint64_t Index;
            size_t       Size;

            // put a item here for simplity
            // the Texture part is nullptr, which should be of initialization by LoadResource()
            EmoticonItem  Item;
        };

    private:
        // for compatible to libzip v-0.xx.x
        struct zip *m_ZIP;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        EmoticonDB()
            : InnDB<uint32_t, EmoticonItem, ResMaxN>()
            , m_ZIP(nullptr)
            , m_ZIPItemInfoCache()
        {}

        virtual ~EmoticonDB()
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

            zip_int64_t nCount = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
            if(nCount > 0){
                for(zip_uint64_t nIndex = 0; nIndex < (zip_uint64_t)(nCount); ++nIndex){
                    struct zip_stat stZIPStat;
                    if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                        if(true
                                && stZIPStat.valid & ZIP_STAT_INDEX
                                && stZIPStat.valid & ZIP_STAT_SIZE
                                && stZIPStat.valid & ZIP_STAT_NAME){

                            // 1. for key, the last byte will always be zero
                            uint32_t nKey = (HexString::ToHex<uint32_t, 4>(stZIPStat.name) & 0XFFFFFF00);

                            // 2. for FPS
                            int nFPS = (int)HexString::ToHex<uint8_t, 1>(stZIPStat.name + 8);

                            // 3. for frame W
                            int nFW = (int)HexString::ToHex<uint16_t, 2>(stZIPStat.name + 10);

                            // 4. for frame H
                            int nFH = (int)HexString::ToHex<uint16_t, 2>(stZIPStat.name + 14);

                            // 5. for frame H1
                            int nH1 = (int)HexString::ToHex<uint16_t, 2>(stZIPStat.name + 18);

                            EmoticonItem stItem {nullptr, nFW, nFH, nH1, nFPS};
                            m_ZIPItemInfoCache[nKey] = {stZIPStat.index, (size_t)(stZIPStat.size), stItem};
                        }
                    }
                }
            }

            return Valid();
        }

    public:
        SDL_Texture *Retrieve(uint32_t nKey,
                int *pSrcX, int *pSrcY,
                int *pSrcW, int *pSrcH,
                int *pH1,
                int *pFPS,
                int *pFrameCount)
        {
            EmoticonItem stItem;
            if(!this->RetrieveResource(nKey & 0XFFFFFF00, &stItem)){
                return nullptr;
            }

            int nW = 0;
            int nH = 0;
            SDL_QueryTexture(stItem.Texture, nullptr, nullptr, &nW, &nH);

            int nCountX = nW / stItem.FrameW;
            int nCountY = nH / stItem.FrameH;

            int nFrameIndex = (int)(nKey & 0X000000FF);

            if(pSrcX      ){ *pSrcX       = (nFrameIndex % nCountX) * stItem.FrameW; }
            if(pSrcY      ){ *pSrcY       = (nFrameIndex / nCountX) * stItem.FrameH; }
            if(pSrcW      ){ *pSrcW       = stItem.FrameW;                           }
            if(pSrcH      ){ *pSrcW       = stItem.FrameH;                           }
            if(pH1        ){ *pH1         = stItem.FrameH1;                          }
            if(pFPS       ){ *pFPS        = stItem.FPS;                              }
            if(pFrameCount){ *pFrameCount = nCountX * nCountY;                       }

            return stItem.Texture;
        }

        SDL_Texture *Retrieve(uint8_t nSet, uint16_t nSubset, uint8_t nIndex,
                int *pSrcX, int *pSrcY,
                int *pSrcW, int *pSrcH,
                int *pFPS,
                int *pH1,
                int *pFrameCount)
        {
            uint32_t nKey = ((uint32_t)(nSet) << 24) + (uint32_t)(nSubset << 8) + (uint32_t)(nIndex);
            return Retrieve(nKey, pSrcX, pSrcY, pSrcW, pSrcH, pH1, pFPS, pFrameCount);
        }

    public:
        virtual std::tuple<EmoticonItem, size_t> LoadResource(uint32_t nKey)
        {
            EmoticonItem stItem {nullptr, 0, 0, 0, 0};
            if(auto pItemRecord = m_ZIPItemInfoCache.find(nKey); pItemRecord != m_ZIPItemInfoCache.end()){
                stItem = pItemRecord->second.Item;
                if(auto fp = zip_fopen_index(m_ZIP, pItemRecord->second.Index, ZIP_FL_UNCHANGED)){

                    // 1. get resource data size to extend buffer
                    std::vector<uint8_t> stBuf;
                    size_t nSize = pItemRecord->second.Size;

                    // 2. dump the data to buffer
                    //    then use sdl to build the texture
                    stBuf.resize(nSize);
                    if(nSize == (size_t)(zip_fread(fp, stBuf.data(), nSize))){
                        extern SDLDevice *g_SDLDevice;
                        stItem.Texture = g_SDLDevice->CreateTexture((const uint8_t *)(stBuf.data()), nSize);
                    }

                    // 3. free the zip item desc
                    //    no matter we decode succeeds or not
                    zip_fclose(fp);
                }
            }
            return {stItem, stItem.Texture ? 1 : 0};
        }

        virtual void FreeResource(EmoticonItem &rstItem)
        {
            if(rstItem.Texture){
                SDL_DestroyTexture(rstItem.Texture);
                rstItem.Texture = nullptr;
            }
        }
};

using EmoticonDBN = EmoticonDB<512>;
