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
#include "hexstr.hpp"
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
//  [0 ~ 1] [2 ~ 5] [6 ~ 7] [8 ~ 9]  [10 ~ 11] [12 ~ 15] [16 ~ 19] [20 ~ 23].PNG
//
//  ESet    ESubset EFrame  FrameCnt FPS       FrameW    FrameH    FrameH1
//  1Byte   2Bytes  1Byte   1Byte    1Byte     2Bytes    2Bytes    2Bytes
//                  +-----
//                  ^
//                  |
//                  +------- always set it as zero
//                             and when retrieving we can just plus the frame index here

struct emojiEntry
{
    SDL_Texture *Texture;
    int          FrameW;
    int          FrameH;
    int          FrameH1;
    int          FPS;
    int          frameCount;
};

class emoticonDB: public innDB<uint32_t, emojiEntry>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        static uint32_t U32Key(uint8_t emojiSet, uint16_t emojiSubset)
        {
            return (to_u32(emojiSet) << 24) | (to_u32(emojiSubset) << 8);
        }

        static uint32_t U32Key(uint8_t emojiSet, uint16_t emojiSubset, uint8_t emojiIndex)
        {
            return U32Key(emojiSet, emojiSubset) | to_u32(emojiIndex);
        }

    public:
        emoticonDB()
            : innDB<uint32_t, emojiEntry>(1024)
            , m_zsdbPtr()
        {}

    public:
        bool Load(const char *szEmojiDBName)
        {
            try{
                m_zsdbPtr = std::make_unique<ZSDB>(szEmojiDBName);
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        SDL_Texture *Retrieve(uint32_t key,
                int *pSrcX, int *pSrcY,
                int *pSrcW, int *pSrcH,
                int *pH1,
                int *pFPS,
                int *pFrameCount)
        {
            emojiEntry entry;
            if(!this->RetrieveResource(key & 0XFFFFFF00, &entry)){
                return nullptr;
            }

            int nW = 0;
            int nH = 0;
            SDL_QueryTexture(entry.Texture, nullptr, nullptr, &nW, &nH);

            const int nCountX = nW / entry.FrameW;
            const int nFrameIndex = to_d(key & 0X000000FF);

            if(pSrcX      ){ *pSrcX       = (nFrameIndex % nCountX) * entry.FrameW; }
            if(pSrcY      ){ *pSrcY       = (nFrameIndex / nCountX) * entry.FrameH; }
            if(pSrcW      ){ *pSrcW       = entry.FrameW;                           }
            if(pSrcH      ){ *pSrcH       = entry.FrameH;                           }
            if(pH1        ){ *pH1         = entry.FrameH1;                          }
            if(pFPS       ){ *pFPS        = entry.FPS;                              }
            if(pFrameCount){ *pFrameCount = entry.frameCount;                       }

            return entry.Texture;
        }

        SDL_Texture *Retrieve(uint8_t nSet, uint16_t nSubset, uint8_t nIndex,
                int *pSrcX, int *pSrcY,
                int *pSrcW, int *pSrcH,
                int *pFPS,
                int *pH1,
                int *pFrameCount)
        {
            uint32_t nKey = (to_u32(nSet) << 24) + to_u32(nSubset << 8) + to_u32(nIndex);
            return Retrieve(nKey, pSrcX, pSrcY, pSrcW, pSrcH, pH1, pFPS, pFrameCount);
        }

    public:
        virtual std::tuple<emojiEntry, size_t> loadResource(uint32_t nKey)
        {
            char keyString[16];
            std::vector<uint8_t> dataBuf;
            emojiEntry entry {nullptr, 0, 0, 0, 0, 0};

            if(auto fileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(nKey, keyString, true), 8, &dataBuf); fileName && (std::strlen(fileName) >= 22)){
                entry.frameCount = (int)hexstr::to_hex< uint8_t, 1>(fileName +  8);
                entry.FPS        = (int)hexstr::to_hex< uint8_t, 1>(fileName + 10);
                entry.FrameW     = (int)hexstr::to_hex<uint16_t, 2>(fileName + 12);
                entry.FrameH     = (int)hexstr::to_hex<uint16_t, 2>(fileName + 16);
                entry.FrameH1    = (int)hexstr::to_hex<uint16_t, 2>(fileName + 20);

                extern SDLDevice *g_sdlDevice;
                entry.Texture = g_sdlDevice->CreateTexture(dataBuf.data(), dataBuf.size());
            }
            return {entry, entry.Texture ? 1 : 0};
        }

        virtual void freeResource(emojiEntry &rstEntry)
        {
            if(rstEntry.Texture){
                SDL_DestroyTexture(rstEntry.Texture);
                rstEntry.Texture = nullptr;
            }
        }
};
