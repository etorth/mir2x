#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientSandCactus: public ClientMonster
{
    public:
        ClientSandCactus(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        bool onActionAttack(const ActionNode &) override;

    protected:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const
        {
            switch(motion){
                case MOTION_MON_STAND  : return {.count =  1};
                case MOTION_MON_ATTACK0: return {.count = 10};
                case MOTION_MON_HITTED : return {.count =  2};
                case MOTION_MON_DIE    : return {.count = 10};
                case MOTION_MON_ATTACK1: return {.count =  6};
                case MOTION_MON_SPELL0 : return {.count = 10};
                default                : return {};
            }
        }
};
