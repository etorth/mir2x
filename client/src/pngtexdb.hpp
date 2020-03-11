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
#include <vector>
#include <memory>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstring.hpp"
#include "sdldevice.hpp"

struct PNGTexEntry
{
    SDL_Texture *Texture;
};

class PNGTexDB: public InnDB<uint32_t, PNGTexEntry>
{
    private:
        std::unique_ptr<ZSDB> m_ZSDBPtr;

    public:
        PNGTexDB(size_t nResMax)
            : InnDB<uint32_t, PNGTexEntry>(nResMax)
            , m_ZSDBPtr()
        {}

    public:
        bool Load(const char *szPNGTexDBName)
        {
            try{
                m_ZSDBPtr = std::make_unique<ZSDB>(szPNGTexDBName);
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        SDL_Texture *Retrieve(uint32_t nKey)
        {
            if(PNGTexEntry stEntry {nullptr}; this->RetrieveResource(nKey, &stEntry)){
                return stEntry.Texture;
            }
            return nullptr;
        }

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage));
        }

    public:
        virtual std::tuple<PNGTexEntry, size_t> LoadResource(uint32_t nKey)
        {
            char szKeyString[16];
            PNGTexEntry stEntry {nullptr};

            if(std::vector<uint8_t> stBuf; m_ZSDBPtr->Decomp(HexString::ToString<uint32_t, 4>(nKey, szKeyString, true), 8, &stBuf)){
                extern SDLDevice *g_SDLDevice;
                stEntry.Texture = g_SDLDevice->CreateTexture(stBuf.data(), stBuf.size());
            }
            return {stEntry, stEntry.Texture ? 1 : 0};
        }

        virtual void FreeResource(PNGTexEntry &rstEntry)
        {
            if(rstEntry.Texture){
                SDL_DestroyTexture(rstEntry.Texture);
                rstEntry.Texture = nullptr;
            }
        }
};
