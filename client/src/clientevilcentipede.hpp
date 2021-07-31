#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientstandmonster.hpp"

class ClientEvilCentipede: public ClientStandMonster
{
    public:
        ClientEvilCentipede(uint64_t, ProcessRun *, const ActionNode &);

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_STAND  : return {.count =  4};
                    case MOTION_MON_ATTACK0: return {.count =  6};
                    case MOTION_MON_HITTED : return {.count =  2};
                    case MOTION_MON_SPAWN : return {.count = 10};
                    default                : return {};
                }
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
                    case MOTION_MON_SPAWN:
                        {
                            return
                            {
                                .gfxMotionID = MOTION_MON_SPAWN,
                                .begin   =  9,
                                .count   = 10,
                                .reverse = true,
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
        bool onActionHitted(const ActionNode &) override;

    public:
        bool canFocus(int pointX, int pointY) const override
        {
            return ClientCreature::canFocus(pointX, pointY) && m_standMode;
        }
};
