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

ClientTaoDog::ClientTaoDog(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientMonster(uid, proc, action)
{
    fflassert(isMonster(u8"神兽"));
    switch(action.type){
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_APPEAR,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = false;
                break;
            }
        case ACTION_STAND:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = action.extParam.stand.dog.standMode;
                break;
            }
        case ACTION_HITTED:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_HITTED,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = action.extParam.hitted.dog.standMode;
                break;
            }
        case ACTION_DIE:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_DIE,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = action.extParam.die.dog.standMode;
                break;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = m_processRun->getAimDirection(action, DIR_UP),
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_SPACEMOVE1:
        case ACTION_SPACEMOVE2:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                // TODO use crowling state
                //      server is supposed to send an ACTION_STAND immediately to fix the standMode
                m_standMode = false;
                break;
            }
        default:
            {
                throw fflerror("Taodog get invalid initial action: %s", actionName(action.type));
            }
    }
}

void ClientTaoDog::addActionTransf()
{
    const auto [endX, endY, endDir] = motionEndGLoc(END_FORCED);
    m_forceMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_SPECIAL,
        .direction = endDir,
        .x = endX,
        .y = endY,
    }));

    m_forceMotionQueue.back()->addUpdate(true, [this](MotionNode *) -> bool
    {
        m_standMode = !m_standMode;
        return true;
    });
}

bool ClientTaoDog::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != action.extParam.stand.dog.standMode){
        addActionTransf();
    }
    return ClientMonster::onActionStand(action);
}

bool ClientTaoDog::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forceMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_APPEAR,
        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientTaoDog::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.dog.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientTaoDog::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }

    const auto [endX, endY, endDir] = motionEndGLoc(END_FORCED);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, endDir),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addUpdate(false, [this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 5){
            return false;
        }

        fflassert(m_standMode);
        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
        {
            u8"神兽_喷火",
            u8"运行",
            currMotion()->x,
            currMotion()->y,
            currMotion()->direction - DIR_BEGIN,
        }));
        return true;
    });
    return true;
}

bool ClientTaoDog::finalStandMode() const
{
    // don't need to count current status
    // check comments in ClientCannibalPlant::finalStandMode()

    int countTransf = 0;
    for(const auto &motionPtr: m_forceMotionQueue){
        if(motionPtr->type == MOTION_MON_SPECIAL){
            countTransf++;
        }
    }

    return (bool)(countTransf % 2) ? !m_standMode : m_standMode;
}
