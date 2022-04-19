#pragma once
#include "pathf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientstandmonster.hpp"

class ClientSandGhost: public ClientStandMonster
{
    public:
        ClientSandGhost(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientStandMonster(uid, proc)
        {
            fflassert(isMonster(u8"沙鬼"));
            switch(action.type){
                case ACTION_SPAWN:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
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
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.stand.sandGhost.standMode);
                        break;
                    }
                case ACTION_ATTACK:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_ATTACK0,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
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
                            .type = MOTION_MON_SPAWN,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.transf.sandGhost.standModeReq);
                        break;
                    }
                case ACTION_MOVE:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_SPAWN,
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
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
                            .direction = pathf::dirValid(action.direction) ? to_d(action.direction) : DIR_UP,
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

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_SPAWN:
                        {
                            return
                            {
                                .begin = 9,
                                .count = 10,
                                .reverse = true,
                            };
                        }
                    case MOTION_MON_STAND  : return {.count =  4};
                    case MOTION_MON_WALK   : return {.count =  6};
                    case MOTION_MON_ATTACK0: return {.count =  6};
                    case MOTION_MON_HITTED : return {.count =  2};
                    case MOTION_MON_DIE    : return {.count = 10};
                    default                : return {};
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND: return {.gfxMotionID = MOTION_MON_SPAWN, .begin = 9, .count =  1};
                    case MOTION_MON_SPAWN: return {.gfxMotionID = MOTION_MON_SPAWN, .begin = 0, .count = 10};
                    default               : return {};
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
