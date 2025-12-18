#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientBugbatMaggot: public ClientMonster
{
    public:
        ClientBugbatMaggot(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"角蝇"));
            m_currMotion.reset(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .direction = DIR_BEGIN,
                .x = action.x,
                .y = action.y,
            });
        }

    protected:
        bool onActionAttack(const ActionNode &) override
        {
            m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_ATTACK0,
                .direction = DIR_BEGIN,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            }));
            return true;
        }
};
