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
#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <SDL2/SDL.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstring.hpp"
#include "sdldevice.hpp"

struct PNGTexOffEntry
{
    SDL_Texture *Texture;
    int          DX;
    int          DY;
};

class PNGTexOffDB: public innDB<uint32_t, PNGTexOffEntry>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        PNGTexOffDB(size_t nResMax)
            : innDB<uint32_t, PNGTexOffEntry>(nResMax)
            , m_zsdbPtr()
        {}

    public:
        bool Load(const char *szPNGTexOffDBName)
        {
            try{
                m_zsdbPtr = std::make_unique<ZSDB>(szPNGTexOffDBName);
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        SDL_Texture *Retrieve(uint32_t nKey, int *pDX, int *pDY)
        {
            if(PNGTexOffEntry stEntry {nullptr, 0, 0}; this->RetrieveResource(nKey, &stEntry)){
                if(pDX){
                    *pDX = stEntry.DX;
                };
                if(pDY){
                    *pDY = stEntry.DY;
                };
                return stEntry.Texture;
            }
            return nullptr;
        }

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage, int *pDX, int *pDY)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage), pDX, pDY);
        }

    public:
        virtual std::tuple<PNGTexOffEntry, size_t> loadResource(uint32_t nKey)
        {
            char szKeyString[16];
            std::vector<uint8_t> stBuf;
            PNGTexOffEntry stEntry {nullptr, 0, 0};

            if(auto szFileName = m_zsdbPtr->Decomp(HexString::ToString<uint32_t, 4>(nKey, szKeyString, true), 8, &stBuf); szFileName && (std::strlen(szFileName) >= 18)){
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

                stEntry.DX = (szFileName[8] != '0') ? 1 : (-1);
                stEntry.DY = (szFileName[9] != '0') ? 1 : (-1);

                stEntry.DX *= (int)(HexString::ToHex<uint32_t, 2>(szFileName + 10));
                stEntry.DY *= (int)(HexString::ToHex<uint32_t, 2>(szFileName + 14));

                extern SDLDevice *g_SDLDevice;
                stEntry.Texture = g_SDLDevice->CreateTexture(stBuf.data(), stBuf.size());
            }
            return {stEntry, stEntry.Texture ? 1 : 0};
        }

        virtual void freeResource(PNGTexOffEntry &rstEntry)
        {
            if(rstEntry.Texture){
                SDL_DestroyTexture(rstEntry.Texture);
                rstEntry.Texture = nullptr;
            }
        }
};
