/*
 * =====================================================================================
 *
 *       Filename: clientcreature.cpp
 *        Created: 08/31/2015 10:45:48 PM
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

#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "log.hpp"
#include "mathf.hpp"
#include "motion.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "ascendstr.hpp"
#include "processrun.hpp"
#include "magicrecord.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "clientcreature.hpp"

extern Log *g_log;

bool ClientCreature::advanceMotionFrame(int addFrame)
{
    const auto frameCount = motionFrameCount(m_currMotion->type, m_currMotion->direction);
    if(frameCount <= 0){
        return false;
    }

    m_currMotion->frame = (m_currMotion->frame + addFrame  ) % frameCount;
    m_currMotion->frame = (m_currMotion->frame + frameCount) % frameCount;
    return true;
}

void ClientCreature::updateAttachMagic(double ms)
{
    for(size_t i = 0; i < m_attachMagicList.size();){
        m_attachMagicList[i]->update(ms);
        if(m_attachMagicList[i]->done()){
            std::swap(m_attachMagicList[i], m_attachMagicList.back());
            m_attachMagicList.pop_back();
        }
        else{
            i++;
        }
    }
}

void ClientCreature::updateHealth(int hp, int hpMax)
{
    if(const auto diff = hp - HP()){
        if(HP() > 0){
            if(m_processRun){
                const int pixelX = x() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2;
                const int pixelY = y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;
                m_processRun->addAscendStr(ASCENDSTR_NUM0, diff, pixelX, pixelY);
            }
        }
    }

    m_HP = hp;
    m_maxHP = hpMax;
}

bool ClientCreature::deadFadeOut()
{
    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(!m_currMotion->extParam.die.fadeOut){
                    m_currMotion->extParam.die.fadeOut = 1;
                }
                return true;
            }
        default:
            {
                return false;
            }
    }
}

bool ClientCreature::alive() const
{
    if(!motionValid(*m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                return false;
            }
        default:
            {
                return true;
            }
    }
}

bool ClientCreature::active() const
{
    if(!motionValid(*m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(auto frameCount = motionFrameCount(m_currMotion->type, m_currMotion->direction); frameCount > 0){
                    return m_currMotion->frame < (frameCount - 1);
                }
                throw fflerror("invalid motion detected");
            }
        default:
            {
                return true;
            }
    }
}

bool ClientCreature::visible() const
{
    if(!motionValid(*m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion->type){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(const auto frameCount = motionFrameCount(m_currMotion->type, m_currMotion->direction); frameCount > 0){
                    return (m_currMotion->frame < (frameCount - 1)) || (m_currMotion->extParam.die.fadeOut < 255);
                }
                throw fflerror("invalid motion detected");
            }
        default:
            {
                return true;
            }
    }
}

std::unique_ptr<MotionNode> ClientCreature::makeIdleMotion() const
{
    return std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = [this]() -> int
        {
            switch(type()){
                case UID_PLY: return MOTION_STAND;
                case UID_NPC: return MOTION_NPC_ACT;
                case UID_MON: return MOTION_MON_STAND;
                default: throw fflerror("invalid ClientCreature: %s", uidf::getUIDString(UID()).c_str());
            }
        }(),

        .direction = m_currMotion->direction,
        .x = m_currMotion->endX,
        .y = m_currMotion->endY
    });
}

double ClientCreature::currMotionDelay() const
{
    auto speed = currMotion()->speed;
    speed = (std::max<int>)(SYS_MINSPEED, speed);
    speed = (std::min<int>)(SYS_MAXSPEED, speed);

    return (1000.0 / SYS_DEFFPS) * (100.0 / speed);
}

void ClientCreature::querySelf()
{
    m_lastQuerySelf = SDL_GetTicks();
    m_processRun->queryCORecord(UID());
}
