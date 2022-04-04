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
    : ClientStandMonster(uid, proc)
{
    fflassert(isMonster(u8"神兽"));
    switch(action.type){
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_SPECIAL,
                    .seq = rollMotionSeq(),
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
                    .seq = rollMotionSeq(),
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
                    .seq = rollMotionSeq(),
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
                    .seq = rollMotionSeq(),
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
                    .seq = rollMotionSeq(),
                    .direction = m_processRun->getAimDirection(action, DIR_UP),
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPACEMOVE:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .seq = rollMotionSeq(),
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.aimX,
                    .y = action.aimY,
                });

                // TODO use crowling state
                //      next ACTION_STAND/ACTION_MOVE will fix it immdiately
                m_standMode = false;
                break;
            }
        default:
            {
                throw fflerror("Taodog get invalid initial action: %s", actionName(action.type));
            }
    }
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
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_SPAWN,
        .seq = rollMotionSeq(),
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

    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .seq = rollMotionSeq(),
        .direction = m_processRun->getAimDirection(action, endDir),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
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
