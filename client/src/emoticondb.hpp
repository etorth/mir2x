/*
 * =====================================================================================
 *
 *       Filename: emoticondb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 09/07/2017 00:32:59
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

template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class EmoticonDB: public InnDB<uint32_t, EmoticonItem, LCDeepN, LCLenN, ResMaxN>
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
        std::vector<uint8_t> m_Buf;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        EmoticonDB()
            : InnDB<uint32_t, EmoticonItem, LCDeepN, LCLenN, ResMaxN>()
            , m_ZIP(nullptr)
            , m_Buf()
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
            if(!m_ZIP){ return false; }

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
        void RetrieveItem(uint32_t nKey, EmoticonItem *pItem, const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            if(pItem){
                // InnRetrieve always return true;
                // we assume nKey == nKey & 0XFFFFFF00
                this->InnRetrieve(nKey, pItem, fnLinearCacheKey, nullptr);
            }
        }

    public:
        // how to load a texture from the database
        // for all pure virtual function required in class InnDB;
        virtual EmoticonItem LoadResource(uint32_t nKey)
        {
            // null resource desc
            EmoticonItem stItem {nullptr, 0, 0, 0, 0};

            auto pItemRecord = m_ZIPItemInfoCache.find(nKey);
            if(pItemRecord != m_ZIPItemInfoCache.end()){
                stItem = pItemRecord->second.Item;

                if(auto fp = zip_fopen_index(m_ZIP, pItemRecord->second.Index, ZIP_FL_UNCHANGED)){

                    // 1. get resource data size to extend buffer
                    size_t nSize = pItemRecord->second.Size;
                    m_Buf.resize(nSize);

                    // 2. dump the data to buffer
                    //    then use sdl to build the texture
                    if(nSize == (size_t)(zip_fread(fp, &(m_Buf[0]), nSize))){
                        extern SDLDevice *g_SDLDevice;
                        stItem.Texture = g_SDLDevice->CreateTexture((const uint8_t *)(&(m_Buf[0])), nSize);
                    }

                    // 3. free the zip item desc
                    //    no matter we decode succeeds or not
                    zip_fclose(fp);
                }
            }

            return stItem;
        }

        void FreeResource(EmoticonItem &stItem)
        {
            if(stItem.Texture){
                SDL_DestroyTexture(stItem.Texture);
            }
        }
};
