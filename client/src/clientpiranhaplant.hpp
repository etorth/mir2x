/*
 * =====================================================================================
 *
 *       Filename: clientpiranhaplant.hpp
 *        Created: 08/31/2015 08:26:19
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <optional>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientPiranhaPlant: public ClientMonster
{
    private:
        bool m_standMode;

    public:
        ClientPiranhaPlant(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc)
        {
            fflassert(isMonster(u8"食人花"));
            switch(action.type){
                case ACTION_SPAWN:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_STAND,
                            .direction = DIR_BEGIN,
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
                            .direction = DIR_BEGIN,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.stand.piranhaPlant.standMode);
                        break;
                    }
                case ACTION_ATTACK:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_ATTACK0,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_BEGIN,
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
                            .direction = DIR_BEGIN,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = (bool)(action.extParam.transf.piranhaPlant.standModeReq);
                        break;
                    }
                case ACTION_HITTED:
                    {
                        m_currMotion.reset(new MotionNode
                        {
                            .type = MOTION_MON_HITTED,
                            .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_BEGIN,
                            .x = action.x,
                            .y = action.y,
                        });

                        m_standMode = true; // can't be hitted if stay in the soil
                        break;
                    }
                default:
                    {
                        throw bad_reach();
                    }
            }
        }

    public:
        FrameSeq motionFrameSeq(int motion, int direction) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_STAND:
                        {
                            if(direction == DIR_BEGIN){
                                return {.count = 4};
                            }
                            return {};
                        }
                    case MOTION_MON_APPEAR:
                        {
                            return
                            {
                                .begin = 7,
                                .count = 8,
                                .reverse = true,
                            };
                        }
                    case MOTION_MON_ATTACK0: return {.count = 6};
                    case MOTION_MON_HITTED : return {.count = 2};
                    case MOTION_MON_DIE    : return {.count =10};
                    default                : return {};
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND : return {.begin = 7, .count = 1};
                    case MOTION_MON_APPEAR: return {.begin = 0, .count = 8};
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

    private:
        void addActionTransf();

    protected:
        std::unique_ptr<MotionNode> makeIdleMotion() const override
        {
            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_STAND,
                .direction = DIR_BEGIN,
                .x = m_currMotion->endX,
                .y = m_currMotion->endY,
            });
        }

    protected:
        bool finalStandMode() const;

    protected:
        int gfxID(int, int) const override;
};
