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
#include "clientcreature.hpp"

extern Log *g_Log;

bool ClientCreature::advanceMotionFrame(int addFrame)
{
    const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
    if(frameCount <= 0){
        return false;
    }

    m_currMotion.frame = (m_currMotion.frame + addFrame  ) % frameCount;
    m_currMotion.frame = (m_currMotion.frame + frameCount) % frameCount;
    return true;
}

bool ClientCreature::updateMotion(bool looped)
{
    const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
    if(frameCount <= 0){
        return false;
    }

    if(looped || (m_currMotion.frame < (frameCount - 1))){
        return advanceMotionFrame(1);
    }
    return moveNextMotion();
}

void ClientCreature::updateAttachMagic(double ms)
{
    for(size_t i = 0; i < m_attachMagicList.size();){
        m_attachMagicList[i]->Update(ms);
        if(m_attachMagicList[i]->Done()){
            std::swap(m_attachMagicList[i], m_attachMagicList.back());
            m_attachMagicList.pop_back();
        }else{
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
    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(!m_currMotion.fadeOut){
                    m_currMotion.fadeOut = 1;
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
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion.motion){
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
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction); frameCount > 0){
                    return m_currMotion.frame < (frameCount - 1);
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
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid motion detected");
    }

    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction); frameCount > 0){
                    return (m_currMotion.frame < (frameCount - 1)) || (m_currMotion.fadeOut < 255);
                }
                throw fflerror("invalid motion detected");
            }
        default:
            {
                return true;
            }
    }
}

MotionNode ClientCreature::makeMotionIdle() const
{
    return MotionNode
    {
        [this]() -> int
        {
            switch(type()){
                case UID_PLY: return MOTION_STAND;
                case UID_NPC: return MOTION_NPC_ACT;
                case UID_MON: return MOTION_MON_STAND;
                default: throw fflerror("invalid ClientCreature: %s", uidf::getUIDString(UID()));
            }
        }(),

        0,
        m_currMotion.direction,
        m_currMotion.speed,
        m_currMotion.endX,
        m_currMotion.endY
    };
}

bool ClientCreature::addAttachMagic(int magicID, int magicParam, int magicStage)
{
    // check if type is u8"附着"
    // otherwise we shouldn't use AttachMagic

    const auto &mr = DBCOM_MAGICRECORD(magicID);
    if(!mr){
        return false;
    }

    for(size_t i = 0;; ++i){
        const auto &ge = mr.GetGfxEntry(i);
        if(!ge){
            // scan all GE and done
            // can't find the stage, stop here
            break;
        }

        if(ge.Stage == magicStage){
            switch(ge.Type){
                case EGT_BOUND:
                    {
                        m_attachMagicList.emplace_back(std::make_unique<AttachMagic>(magicID, magicParam, magicStage));
                        return true;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}

double ClientCreature::currMotionDelay() const
{
    auto speed = currMotion().speed;
    speed = (std::max<int>)(SYS_MINSPEED, speed);
    speed = (std::min<int>)(SYS_MAXSPEED, speed);

    return (1000.0 / SYS_DEFFPS) * (100.0 / speed);
}

void ClientCreature::querySelf()
{
    m_lastQuerySelf = SDL_GetTicks();
    m_processRun->queryCORecord(UID());
}
