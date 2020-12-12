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

#include "processrun.hpp"
#include "clienttaodog.hpp"

bool ClientTaoDog::onActionSpawn(const ActionNode &action)
{
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
    if(m_stand == standReq){
        return true;
    }

    const auto [x, y, dir] = [this]() -> std::array<int, 3>
    {
        if(!m_forceMotionQueue.empty()){
            return {m_forceMotionQueue.back()->endX, m_forceMotionQueue.back()->endY, m_forceMotionQueue.back()->direction};
        }
        return {m_currMotion->endX, m_currMotion->endY, m_currMotion->direction};
    }();

    m_stand = standReq;
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
    const auto [endX, endY] = motionEndLocation(END_FORCED);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.AimUID)){
        const auto nX   = coPtr->x();
        const auto nY   = coPtr->y();
        const auto nDir = PathFind::GetDirection(action.X, action.Y, nX, nY);

        if(!(nDir >= DIR_BEGIN && nDir < DIR_END)){
            throw fflerror("invalid direction: %d", nDir);
        }

        auto motionPtr = new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = nDir,
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

            if(m_stand){
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
