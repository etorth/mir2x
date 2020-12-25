/*
 * =====================================================================================
 *
 *       Filename: clienttaodog.cpp
 *        Created: 08/31/2015 08:26:19
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

#include "fflerror.hpp"
#include "processrun.hpp"
#include "clienttaodog.hpp"

void ClientTaoDog::addActionTransf(bool /* standMode */)
{
    const auto [x, y, dir] = motionEndLocation(END_CURRENT);
    auto motionPtr = new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = dir,
        .x = x,
        .y = y,
    };

    motionPtr->onUpdate = [motionPtr, lastFrame = (int)(-1), this]() mutable
    {
        if(lastFrame != motionPtr->frame && motionPtr->frame == 0){
            m_standMode = true;
        }
        lastFrame = motionPtr->frame;
    };

    m_forceMotionQueue.push_front(std::unique_ptr<MotionNode>(motionPtr));
}

bool ClientTaoDog::onActionSpawn(const ActionNode &action)
{
    if(!m_forceMotionQueue.empty()){
        throw fflerror("found motion before spawn: %s", uidf::getUIDString(UID()).c_str());
    }

    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = [&action]() -> int
        {
            if(directionValid(action.direction)){
                return action.direction;
            }
            return DIR_UP;
        }(),

        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientTaoDog::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.dog.standMode);
    if(m_standMode == standReq){
        return true;
    }

    // change shape immediately
    // don't wait otherwise there may has transf in the forced motion queue

    addActionTransf(m_standMode);
    return true;
}

bool ClientTaoDog::onActionAttack(const ActionNode &action)
{
    if(!m_standMode){
        bool hasTransf = false;
        for(const auto &motionPtr: m_forceMotionQueue){
            if(motionPtr->type == MOTION_MON_APPEAR){
                hasTransf = true;
            }
        }

        if(!hasTransf){
            addActionTransf(true);
        }
    }

    const auto [endX, endY, endDir] = motionEndLocation(END_FORCED);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.aimUID)){
        auto motionPtr = new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2<int>(nX, nY, action.x, action.y) == 0){
                    return endDir;
                }
                return PathFind::GetDirection(action.x, action.y, nX, nY);
            }(),
            .x = action.x,
            .y = action.y,
        };

        motionPtr->onUpdate = [motionPtr, lastFrame = (int)(-1), this]() mutable
        {
            if(lastFrame == motionPtr->frame){
                return;
            }

            lastFrame = motionPtr->frame;
            if(motionPtr->frame != 5){
                return;
            }

            if(!m_standMode){
                throw fflerror("ClientTaoDog attacks while not standing");
            }

            m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
            {
                u8"神兽-喷火",
                u8"运行",
                currMotion()->x,
                currMotion()->y,
                currMotion()->direction - DIR_BEGIN,
            }));
        };

        m_motionQueue.push_back(std::unique_ptr<MotionNode>(motionPtr));
        return true;
    }
    return false;
}
