/*
 * =====================================================================================
 *
 *       Filename: clientsandcactus.hpp
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
#include "clientsandcactus.hpp"

ClientSandCactus::ClientSandCactus(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientMonster(uid, proc, action)
{
    fflassert(isMonster(u8"沙漠树魔"));
    switch(action.type){
        case ACTION_SPAWN:
        case ACTION_STAND:
        case ACTION_HITTED:
        case ACTION_DIE:
        case ACTION_ATTACK:
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
                throw fflerror("Taodog get invalid initial action: %s", actionName(action.type));
            }
    }
}

bool ClientSandCactus::onActionAttack(const ActionNode &action)
{
    fflassert(action.x == currMotion()->x);
    fflassert(action.y == currMotion()->y);

    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_ATTACK0,
        .direction = m_processRun->getAimDirection(action, currMotion()->direction),
        .x = action.x,
        .y = action.y,
    }));

    m_motionQueue.back()->addUpdate(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
    {
        if(motionPtr->frame < 5){
            return false;
        }

        m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
        {
            u8"沙漠树魔_喷刺",
            u8"运行",

            currMotion()->x * SYS_MAPGRIDXP,
            currMotion()->y * SYS_MAPGRIDYP,

            0,
            (m_currMotion->direction - DIR_BEGIN) * 2,
            20,

            targetUID,
            m_processRun,
        }))->addOnDone([targetUID, this](MagicBase *)
        {
            if(auto coPtr = m_processRun->findUID(targetUID)){
                coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"沙漠树魔_喷刺", u8"结束")));
            }
        });
        return true;
    });
    return true;
}
