#pragma once
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientGasAnt: public ClientMonster
{
    public:
        ClientGasAnt(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"爆毒蚂蚁"));
        }

    protected:
        bool onActionAttack(const ActionNode &action) override
        {
            const auto [endX, endY, endDir] = motionEndGLoc().at(1);
            m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
            m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_ATTACK0,
                .seq = rollMotionSeq(),
                .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                .x = action.x,
                .y = action.y,
            }));

            m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
            {
                if(motionPtr->frame < 2){
                    return false;
                }

                m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
                {
                    u8"爆毒蚂蚁_喷毒",
                    u8"运行",

                    motionPtr->x * SYS_MAPGRIDXP,
                    motionPtr->y * SYS_MAPGRIDYP,

                    0,
                    (motionPtr->direction - DIR_BEGIN) * 2,
                    20,

                    targetUID,
                    m_processRun,
                }));
                return true;
            });
            return true;
        }
};
