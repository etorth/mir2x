#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientNumaWizard: public ClientMonster
{
    public:
        ClientNumaWizard(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"诺玛法老") || isMonster(u8"大法老"));
        }

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int direction) const override
        {
            if(isMonster(u8"大法老") && (motion == MOTION_MON_ATTACK0)){
                return
                {
                    .gfxMotionID = MOTION_MON_SPELL0,
                    .count = 6,
                };
            }
            return ClientMonster::getFrameGfxSeq(motion, direction);
        }

    protected:
        bool onActionAttack(const ActionNode &action)
        {
            if(isMonster(u8"诺玛法老")){
                return onActionAttack_fireBall(action);
            }
            else if(isMonster(u8"大法老")){
                return onActionAttack_thunderBolt(action);
            }
            else{
                throw fflerror("invalid monster: %s", to_cstr(monsterName()));
            }
        }

    private:
        bool onActionAttack_fireBall   (const ActionNode &action);
        bool onActionAttack_thunderBolt(const ActionNode &action);
};
