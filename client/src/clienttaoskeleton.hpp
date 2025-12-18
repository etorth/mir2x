#pragma once
#include "pathf.hpp"
#include "clientmonster.hpp"

class ClientTaoSkeleton: public ClientMonster
{
    public:
        ClientTaoSkeleton(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"变异骷髅"));
            m_currMotion.reset(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .direction = [&action]() -> int
                {
                    if(action.type == ACTION_SPAWN){
                        return DIR_DOWNLEFT;
                    }
                    else if(pathf::dirValid(action.direction)){
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
