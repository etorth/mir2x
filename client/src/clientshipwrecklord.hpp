#pragma once
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientShipwreckLord: public ClientMonster
{
    public:
        ClientShipwreckLord(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"霸王教主"));
        }

    protected:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            switch(motion){
                case MOTION_MON_STAND  : return {.count =  4};
                case MOTION_MON_WALK   : return {.count =  6};
                case MOTION_MON_ATTACK0: return {.count = 10};
                case MOTION_MON_HITTED : return {.count =  2};
                case MOTION_MON_DIE    : return {.count = 10};
                case MOTION_MON_SPELL0 : return {.count = 10};
                default                : return {};
            }
        }

    protected:
        bool onActionAttack(const ActionNode &) override;
};
