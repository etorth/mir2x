#pragma once
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientWoomaFlamingWarrior: public ClientMonster
{
    public:
        ClientWoomaFlamingWarrior(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"火焰沃玛"));
        }

    protected:
        bool onActionAttack(const ActionNode &action)
        {
            m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_ATTACK0,
                .direction = m_processRun->getAimDirection(action, currMotion()->direction),
                .x = action.x,
                .y = action.y,
            }));

            m_motionQueue.back()->addUpdate(false, [this](MotionNode *motionPtr) -> bool
            {
                if(motionPtr->frame < 3){
                    return false;
                }

                m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                {
                    u8"火焰沃玛_喷火",
                    u8"运行",
                    currMotion()->x,
                    currMotion()->y,
                    currMotion()->direction - DIR_BEGIN,
                }));
                return true;
            });
            return true;
        }
};
