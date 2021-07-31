#pragma once
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientstandmonster.hpp"

class ClientEvilCentipede: public ClientStandMonster
{
    public:
        ClientEvilCentipede(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientStandMonster(uid, proc)
        {
            fflassert(isMonster(u8"触龙神"));
            switch(action.type){
                case ACTION_SPAWN:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = false;
                        break;
                    }
                case ACTION_STAND:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.stand.evilCentipede.standMode);
                        break;
                    }
                case ACTION_ATTACK:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_ATTACK0,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = true;
                        break;
                    }
                case ACTION_TRANSF:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_APPEAR,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.transf.evilCentipede.standModeReq);
                        break;
                    }
                case ACTION_MOVE:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_APPEAR,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = true;
                        break;
                    }
                case ACTION_HITTED:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_HITTED,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = true; // can't be hitted if stay in the soil
                        break;
                    }
                default:
                    {
                        throw fflerror("invalid action: %s", actionName(action));
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
};
