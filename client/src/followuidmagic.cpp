/*
 * =====================================================================================
 *
 *       Filename: followuidmagic.cpp
 *        Created: 12/07/2020 21:19:44
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

#include <cmath>
#include "pathf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"
#include "followuidmagic.hpp"

extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_magicDB;

FollowUIDMagic::FollowUIDMagic(
        const char8_t *magicName,
        const char8_t *magicStage,
        
        int x,
        int y,
        int gfxDirIndex,
        int moveSpeed,

        uint64_t uid,
        ProcessRun *runPtr)
    : MagicBase(magicName, magicStage, gfxDirIndex)
    , m_x(x)
    , m_y(y)
    , m_moveSpeed(moveSpeed)
    , m_uid(uid)
    , m_process(runPtr)
{
    if(!m_process){
        throw fflerror("invalid process pointer: nullptr");
    }

    if(!(m_x >= 0 && m_y >= 0 && m_process->onMap(m_x / SYS_MAPGRIDXP, m_y / SYS_MAPGRIDYP))){
        throw fflerror("invalid pixel on map: x = %d, y = %d", m_x, m_y);
    }

    if(!m_uid){
        throw fflerror("invalid uid: 0");
    }
}

void FollowUIDMagic::update(double ms)
{
    MagicBase::update(ms);

    const auto [dx, dy] = [this]() -> std::tuple<int, int>
    {
        if(const auto coPtr = m_process->findUID(m_uid)){
            const auto [x, y] = coPtr->getTargetBox().center();
            const int xdiff = x - m_x;
            const int ydiff = y - m_y;

            if(xdiff == 0 && ydiff == 0){
                return {0, 0};
            }
            else{
                return pathf::getDirOff(xdiff, ydiff, m_moveSpeed);
            }
        }
        else{
            switch(m_gfxEntry->gfxDirType){
                case 16: return pathf::getDir16Off(m_gfxDirIndex, m_moveSpeed);
                case  8: return pathf::getDir8Off (m_gfxDirIndex, m_moveSpeed);
                case  4: return pathf::getDir4Off (m_gfxDirIndex, m_moveSpeed);
                default: throw fflerror("invalid gfxDirType: %d", m_gfxEntry->gfxDirType);
            }
        }
    }();

    m_x += dx;
    m_y += dy;
}

void FollowUIDMagic::drawViewOff(int viewX, int viewY, bool alpha)
{
    if(m_gfxEntry->gfxID == SYS_TEXNIL){
        return;
    }

    const auto texID = [this]() -> uint32_t
    {
        switch(m_gfxEntry->gfxDirType){
            case  1: return m_gfxEntry->gfxID + frame();
            case  4:
            case  8:
            case 16: return m_gfxEntry->gfxID + frame() + m_gfxDirIndex * m_gfxEntry->gfxIDCount;
            default: throw fflerror("invalid gfxDirType: %d", m_gfxEntry->gfxDirType);
        }
    }();

    int offX = 0;
    int offY = 0;
    if(auto texPtr = g_magicDB->Retrieve(texID, &offX, &offY)){
        SDLDevice::EnableTextureModColor enableModColor(texPtr, colorf::RGBA(0XFF, 0XFF, 0XFF, alpha ? 0X80 : 0XFF));
        SDLDevice::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_SDLDevice->drawTexture(texPtr, m_x - viewX + offX, m_y - viewY + offY);
    }
}

bool FollowUIDMagic::done() const
{
    if(const auto coPtr = m_process->findUID(m_uid); coPtr->getTargetBox().in(m_x, m_y)){
        return true;
    }
    return false;
}
