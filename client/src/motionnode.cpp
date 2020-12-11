/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
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
#include <cinttypes>
#include "log.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "motionnode.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;

void MagicSpellEffect::drawShift(int shiftX, int shiftY, bool alpha)
{
    if(const auto texID = frameTexID(); texID != SYS_TEXNIL){
        int offX = 0;
        int offY = 0;
        if(auto texPtr = g_magicDB->Retrieve(texID, &offX, &offY)){
            SDLDevice::EnableTextureModColor enableModColor(texPtr, colorf::RGBA(0XFF, 0XFF, 0XFF, alpha ? 0X40 : 0XC0));
            SDLDevice::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
            g_sdlDevice->drawTexture(texPtr, shiftX + offX, shiftY + offY);
        }
    }
}

void MotionNode::print() const
{
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::motion      = %s", to_cvptr(this), motionName(type)    );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::direction   = %d", to_cvptr(this), direction           );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::speed       = %d", to_cvptr(this), speed               );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::x           = %d", to_cvptr(this), x                   );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::y           = %d", to_cvptr(this), y                   );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endX        = %d", to_cvptr(this), endX                );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endY        = %d", to_cvptr(this), endY                );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::frame       = %d", to_cvptr(this), frame               );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::onUpdate    = %s", to_cvptr(this), onUpdate ? "1" : "0");
}
