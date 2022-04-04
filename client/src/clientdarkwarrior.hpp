#pragma once
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientDarkWarrior: public ClientMonster
{
    public:
        ClientDarkWarrior(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"暗黑战士"));
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
                .direction = m_processRun->getAimDirection(action, endDir),
                .x = action.x,
                .y = action.y,
            }));

            m_motionQueue.back()->addTrigger(false, [targetUID = action.aimUID, this](MotionNode *motionPtr) -> bool
            {
                if(motionPtr->frame < 5){
                    return false;
                }

                m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
                {
                    u8"暗黑战士_喷刺",
                    u8"运行",

                    currMotion()->x * SYS_MAPGRIDXP,
                    currMotion()->y * SYS_MAPGRIDYP,

                    (m_currMotion->direction - DIR_BEGIN) * 2,
                    (m_currMotion->direction - DIR_BEGIN) * 2,
                    20,

                    targetUID,
                    m_processRun,
                }))->addOnDone([targetUID, proc = m_processRun](BaseMagic *)
                {
                    if(auto coPtr = proc->findUID(targetUID)){
                        coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"沙漠树魔_喷刺", u8"结束")));
                    }
                });
                return true;
            });
            return true;
        }
};
