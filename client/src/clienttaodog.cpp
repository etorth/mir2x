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
            if(action.Direction >= DIR_BEGIN && action.Direction < DIR_END){
                return action.Direction;
            }
            return DIR_UP;
        }(),

        .x = action.X,
        .y = action.Y,
    });
    return true;
}

bool ClientTaoDog::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.ActionParam);
    if(m_standMode == standReq){
        return true;
    }

    const auto [x, y, dir] = motionEndLocation(END_FORCED);

    m_standMode = standReq;
    m_forceMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = dir,
        .x = x,
        .y = y,
    }));
    return true;
}

bool ClientTaoDog::onActionAttack(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndLocation(END_FORCED);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.AimUID)){
        auto motionPtr = new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2(nX, nY, action.X, action.Y) == 0){
                    return endDir;
                }
                return PathFind::GetDirection(action.X, action.Y, nX, nY);
            }(),
            .x = action.X,
            .y = action.Y,
        };

        motionPtr->onUpdate = [motionPtr, lastMotionFrame = (int)(-1), this]() mutable
        {
            if(lastMotionFrame == motionPtr->frame){
                return;
            }

            lastMotionFrame = motionPtr->frame;
            if(motionPtr->frame != 5){
                return;
            }

            if(m_standMode){
                m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"神兽-喷火",
                    u8"运行",
                    currMotion()->x,
                    currMotion()->y,
                    currMotion()->direction - DIR_BEGIN,
                }));
            }
        };

        m_motionQueue.push_back(std::unique_ptr<MotionNode>(motionPtr));
        return true;
    }
    return false;
}
