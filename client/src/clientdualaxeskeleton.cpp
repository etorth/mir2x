/*
 * =====================================================================================
 *
 *       Filename: clientdualaxeskeleton.cpp
 *        Created: 07/31/2021 08:26:19
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
#include "clientdualaxeskeleton.hpp"

ClientDualAxeSkeleton::ClientDualAxeSkeleton(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientMonster(uid, proc, action)
{
    fflassert(isMonster(u8"掷斧骷髅"));
    switch(action.type){
        case ACTION_SPAWN:
        case ACTION_STAND:
        case ACTION_HITTED:
        case ACTION_DIE:
        case ACTION_ATTACK:
        case ACTION_MOVE:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.type) ? to_d(action.type) : DIR_BEGIN,
                    .x = action.x,
                    .y = action.y,
                });
                break;
            }
        default:
            {
                throw fflerror("invalid initial action: %s", actionName(action.type));
            }
    }
}

bool ClientDualAxeSkeleton::onActionAttack(const ActionNode &action)
{
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 4){
            return false;
        }

        const auto gfxDirIndex = currMotion()->direction - DIR_BEGIN;
        m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
        {
            u8"掷斧骷髅_掷斧",
            u8"运行",

            currMotion()->x * SYS_MAPGRIDXP,
            currMotion()->y * SYS_MAPGRIDYP,

            gfxDirIndex,
            gfxDirIndex * 2,
            20,

            targetUID,
            m_processRun,
        }))->addOnDone([targetUID, proc = m_processRun](BaseMagic *)
        {
            // TODO interesting bug, don't directly refer to this->m_processRun
            // an dual-axe-skeleton can throw dual-axe-magic and die immediately, which makes *this* dangling

            if(auto coPtr = proc->findUID(targetUID)){
                coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"掷斧骷髅_掷斧", u8"裂解")));
            }
        });
        return true;
    });
    return true;
}
