/*
 * =====================================================================================
 *
 *       Filename: ascendstr.cpp
 *        Created: 07/20/2017 00:34:13
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
#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "ascendstr.hpp"
#include "sdldevice.hpp"
#include "totype.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AscendStr::AscendStr(int argType, int argValue, int argX, int argY)
    : m_type(argType)
    , m_value(argValue)
    , m_x(argX)
    , m_y(argY)
{
    switch(m_type){
        case ASCENDSTR_MISS:
            {
                m_value = 0;
                break;
            }
        case ASCENDSTR_RED:
        case ASCENDSTR_BLUE:
        case ASCENDSTR_GREEN:
            {
                break;
            }
        default:
            {
                throw fflreach();
            }
    }
}

void AscendStr::draw(int viewX, int viewY)
{
    if(ratio() < 1.0){
        /* */ auto currX = x();
        const auto currY = y();
        const auto currA = to_u8(std::lround(255 * (1.0 - ratio())));

        switch(type()){
            case ASCENDSTR_MISS:
                {
                    if(auto texPtr = g_progUseDB->retrieve(0X03000030)){
                        SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, colorf::RGBA(255, 255, 255, currA));
                        g_sdlDevice->drawTexture(texPtr, currX - viewX, currY - viewY);
                    }
                    break;
                }
            case ASCENDSTR_RED:
            case ASCENDSTR_BLUE:
            case ASCENDSTR_GREEN:
                {
                    if(value()){
                        const uint32_t baseKey = 0X03000000 | ((type() - ASCENDSTR_BEGIN) << 4);
                        if(auto texPtr = g_progUseDB->retrieve(baseKey | ((value() < 0) ? 0X0A : 0X0B))){
                            SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, colorf::RGBA(255, 255, 255, currA));
                            g_sdlDevice->drawTexture(texPtr, currX - viewX, currY - viewY + ((value() < 0) ? 4 : 1));
                            currX += SDLDeviceHelper::getTextureWidth(texPtr);
                        }

                        for(const auto chNum: std::to_string(std::labs(value()))){
                            if(auto texPtr = g_progUseDB->retrieve(baseKey | (chNum - '0'))){
                                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, colorf::RGBA(255, 255, 255, currA));
                                g_sdlDevice->drawTexture(texPtr, currX - viewX, currY - viewY);
                                currX += SDLDeviceHelper::getTextureWidth(texPtr);
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
