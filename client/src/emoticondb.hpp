/*
 * =====================================================================================
 *
 *       Filename: emoticondb.hpp
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
//  1Byte   2Bytes  1Byte   1Byte   2Bytes    2Bytes    2Bytes
//                  ------
//                    ^
//                    |
//                    +------- always set it as zero
//                             and when retrieving we can just plus the frame index here

struct EmojiEntry
{
    SDL_Texture *Texture;
    int          FrameW;
    int          FrameH;
    int          FrameH1;
    int          FPS;
};

class EmoticonDB: public InnDB<uint32_t, EmojiEntry>
{
    private:
        std::unique_ptr<ZSDB> m_ZSDBPtr;

    public:
        static uint32_t U32Key(uint8_t emojiSet, uint16_t emojiSubset)
        {
            return ((uint32_t)(emojiSet) << 24) | ((uint32_t)(emojiSubset) << 8);
        }

        static uint32_t U32Key(uint8_t emojiSet, uint16_t emojiSubset, uint8_t emojiIndex)
        {
            return U32Key(emojiSet, emojiSubset) | (uint32_t)(emojiIndex);
        }

    public:
        EmoticonDB()
            : InnDB<uint32_t, EmojiEntry>(1024)
            , m_ZSDBPtr()
        {}

    public:
        bool Load(const char *szEmojiDBName)
        {
            try{
                m_ZSDBPtr = std::make_unique<ZSDB>(szEmojiDBName);
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        SDL_Texture *Retrieve(uint32_t nKey,
                int *pSrcX, int *pSrcY,
                int *pSrcW, int *pSrcH,
                int *pH1,
                int *pFPS,
                int *pFrameCount)
        {
            EmojiEntry stEntry;
            if(!this->RetrieveResource(nKey & 0XFFFFFF00, &stEntry)){
                return nullptr;
            }

            int nW = 0;
            int nH = 0;
            SDL_QueryTexture(stEntry.Texture, nullptr, nullptr, &nW, &nH);

            int nCountX = nW / stEntry.FrameW;
            int nCountY = nH / stEntry.FrameH;

            int nFrameIndex = (int)(nKey & 0X000000FF);

            if(pSrcX      ){ *pSrcX       = (nFrameIndex % nCountX) * stEntry.FrameW; }
            if(pSrcY      ){ *pSrcY       = (nFrameIndex / nCountX) * stEntry.FrameH; }
            if(pSrcW      ){ *pSrcW       = stEntry.FrameW;                           }
            if(pSrcH      ){ *pSrcW       = stEntry.FrameH;                           }
            if(pH1        ){ *pH1         = stEntry.FrameH1;                          }
            if(pFPS       ){ *pFPS        = stEntry.FPS;                              }
            if(pFrameCount){ *pFrameCount = nCountX * nCountY;                        }

            return stEntry.Texture;
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
        virtual std::tuple<EmojiEntry, size_t> LoadResource(uint32_t nKey)
        {
            char szKeyString[16];
            std::vector<uint8_t> stBuf;
            EmojiEntry stEntry {nullptr, 0, 0, 0, 0};

            if(auto szFileName = m_ZSDBPtr->Decomp(HexString::ToString<uint32_t, 4>(nKey, szKeyString, true), 8, &stBuf); szFileName && (std::strlen(szFileName) >= 22)){
                stEntry.FPS     = (int)HexString::ToHex< uint8_t, 1>(szFileName +  8);
                stEntry.FrameW  = (int)HexString::ToHex<uint16_t, 2>(szFileName + 10);
                stEntry.FrameH  = (int)HexString::ToHex<uint16_t, 2>(szFileName + 14);
                stEntry.FrameH1 = (int)HexString::ToHex<uint16_t, 2>(szFileName + 18);

                extern SDLDevice *g_SDLDevice;
                stEntry.Texture = g_SDLDevice->CreateTexture(stBuf.data(), stBuf.size());
            }
            return {stEntry, stEntry.Texture ? 1 : 0};
        }

        virtual void FreeResource(EmojiEntry &rstEntry)
        {
            if(rstEntry.Texture){
                SDL_DestroyTexture(rstEntry.Texture);
                rstEntry.Texture = nullptr;
            }
        }
};
