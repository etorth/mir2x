/*
 * =====================================================================================
 *
 *       Filename: ascendstr.cpp
 *        Created: 07/20/2017 00:34:13
 *  Last Modified: 07/20/2017 18:04:23
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */
#include <cstdint>
#include <cinttypes>
#include "log.hpp"
#include "ascendstr.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"

AscendStr::AscendStr(int nType, int nValue, int nX, int nY)
    : m_Type(nType)
    , m_Value(nValue)
    , m_X(nX)
    , m_Y(nY)
    , m_Tick(0.0)
{
    switch(m_Type){
        case ASCENDSTR_MISS:
            {
                m_Value = 0;
                break;
            }
        case ASCENDSTR_NUM0:
        case ASCENDSTR_NUM1:
        case ASCENDSTR_NUM2:
            {
                break;
            }
        default:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid AscendStr type: %d", Type());
                break;
            }
    }

    // TODO
    // should I here put checks for (nX, nY) in current map?
}

void AscendStr::Update(double fUpdate)
{
    m_Tick += fUpdate;
}

void AscendStr::Draw(int nViewX, int nViewY)
{
    if(Ratio() < 1.0){
        extern SDLDevice *g_SDLDevice;
        extern PNGTexDBN *g_ProgUseDBN;

        auto nCurrX = X();
        auto nCurrY = Y();
        auto nCurrA = (Uint8)(std::lround(255 * Ratio()));

        switch(Type()){
            case ASCENDSTR_MISS:
                {
                    if(auto pTexture = g_ProgUseDBN->Retrieve(0X03000030)){
                        SDL_SetTextureColorMod(pTexture, nCurrA, nCurrA, nCurrA);
                        g_SDLDevice->DrawTexture(pTexture, nCurrX - nViewX, nCurrY - nViewY);
                    }
                    break;
                }
            case ASCENDSTR_NUM0:
            case ASCENDSTR_NUM1:
            case ASCENDSTR_NUM2:
                {
                    if(Value()){
                        uint32_t nPreKey = 0X03000000 | ((Type() - ASCENDSTR_NUM0) << 4);
                        if(auto pTexture = g_ProgUseDBN->Retrieve(nPreKey | ((Value() < 0) ? 0X0A : 0X0B))){
                            SDL_SetTextureColorMod(pTexture, nCurrA, nCurrA, nCurrA);
                            g_SDLDevice->DrawTexture(pTexture, nCurrX - nViewX, nCurrY - nViewY + ((Value() < 0) ? 4 : 1));

                            int nTextureW;
                            SDL_QueryTexture(pTexture, nullptr, nullptr, &nTextureW, nullptr);
                            nCurrX += nTextureW;
                        }

                        auto szNumStr = std::to_string(std::labs(Value()));
                        for(auto chNum: szNumStr){
                            if(auto pTexture = g_ProgUseDBN->Retrieve(nPreKey | (chNum - '0'))){
                                SDL_SetTextureColorMod(pTexture, nCurrA, nCurrA, nCurrA);
                                g_SDLDevice->DrawTexture(pTexture, nCurrX - nViewX, nCurrY - nViewY);

                                int nTextureW;
                                SDL_QueryTexture(pTexture, nullptr, nullptr, &nTextureW, nullptr);
                                nCurrX += nTextureW;
                            }
                        }
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}
