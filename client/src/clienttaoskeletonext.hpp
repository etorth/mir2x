#pragma once
#include "clientmonster.hpp"

class ClientTaoSkeletonExt: public ClientMonster
{
    public:
        ClientTaoSkeletonExt(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"超强骷髅"));
            m_currMotion.reset(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .seq = rollMotionSeq(),
                .direction = [&action]() -> int
                {
                    if(action.type == ACTION_SPAWN){
                        return DIR_DOWNLEFT;
                    }
                    else if(directionValid(action.direction)){
                        return action.direction;
                    }
                    else{
                        return DIR_UP;
                    }
                }(),

                .x = action.x,
                .y = action.y,
            });
        }
};
