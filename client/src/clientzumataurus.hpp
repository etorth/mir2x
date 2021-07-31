#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientstandmonster.hpp"

class ClientZumaTaurus: public ClientStandMonster
{
    public:
        ClientZumaTaurus(uint64_t, ProcessRun *, const ActionNode &);

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int dir) const override
        {
            if(m_standMode){
                return ClientStandMonster::getFrameGfxSeq(motion, dir);
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND:
                        {
                            return
                            {
                                .gfxMotionID = MOTION_MON_SPAWN,
                                .begin = 0,
                                .count = 1,
                            };
                        }
                    default:
                        {
                            return {};
                        }
                }
            }
        }

    protected:
        bool onActionSpawn (const ActionNode &) override;
        bool onActionStand (const ActionNode &) override;
        bool onActionTransf(const ActionNode &) override;
        bool onActionAttack(const ActionNode &) override;

    public:
        bool canFocus(int pointX, int pointY) const override
        {
            return ClientCreature::canFocus(pointX, pointY) && m_standMode;
        }

    protected:
        void addActionTransf() override;
};
