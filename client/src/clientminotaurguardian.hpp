#pragma once
#include <cstdint>
#include "motion.hpp"
#include "fflerror.hpp"
#include "actionnode.hpp"
#include "processrun.hpp"
#include "clientmonster.hpp"

class ClientMinotaurGuardian: public ClientMonster
{
    public:
        ClientMinotaurGuardian(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"潘夜左护卫") || isMonster(u8"潘夜右护卫"));
        }

    protected:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            switch(motion){
                case MOTION_MON_STAND  : return {.count =  4};
                case MOTION_MON_WALK   : return {.count =  6};
                case MOTION_MON_ATTACK0: return {.count =  6};
                case MOTION_MON_HITTED : return {.count =  2};
                case MOTION_MON_DIE    : return {.count = 10};
                case MOTION_MON_ATTACK1: return {.count =  6};
                default                : return {};
            }
        }

    protected:
        bool onActionAttack(const ActionNode &) override;
};
