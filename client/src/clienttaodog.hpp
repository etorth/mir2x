/*
 * =====================================================================================
 *
 *       Filename: clienttaodog.hpp
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
#include <unordered_map>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientTaoDog: public ClientMonster
{
    private:
        bool m_standMode;

    public:
        ClientTaoDog(uint64_t, ProcessRun *, const ActionNode &);

    public:
        FrameSeq motionFrameSeq(int motion, int) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_STAND  : return {.count =  4};
                    case MOTION_MON_WALK   : return {.count =  6};
                    case MOTION_MON_HITTED : return {.count =  2};
                    case MOTION_MON_DIE    : return {.count = 10};
                    case MOTION_MON_ATTACK0: return {.count =  6};
                    case MOTION_MON_SPECIAL:
                        {
                            // from crawling to stand
                            // need gfx redirect, gfx index is MOTION_MON_APPEAR
                            // user MOTION_MON_SPECIAL here to simplify finalStandMode() to count
                            return {.count = 10};
                        }
                    default:
                        {
                            return {};
                        }
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND  : return {.count =  4};
                    case MOTION_MON_WALK   : return {.count =  6};
                    case MOTION_MON_HITTED : return {.count =  2};
                    case MOTION_MON_DIE    : return {.count = 10};
                    case MOTION_MON_APPEAR : return {.count = 10}; // from non to crawling
                    case MOTION_MON_SPECIAL:
                        {
                            // from stand to crawling
                            // need gfxID redirect and lookID redirect
                            return
                            {
                                .begin = 9,
                                .count = 10,
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

    public:
        int lookID() const override
        {
            if(m_standMode){
                return 0X5A;
            }
            else{
                if(m_currMotion->type == MOTION_MON_SPECIAL){
                    return 0X5A;
                }
                else{
                    return 0X59;
                }
            }
        }

    protected:
        bool onActionSpawn (const ActionNode &) override;
        bool onActionTransf(const ActionNode &) override;
        bool onActionAttack(const ActionNode &) override;

    public:
        bool visible() const override
        {
            return ClientCreature::active();
        }

    private:
        void addActionTransf();

    protected:
        bool finalStandMode() const;

    protected:
        int gfxID(int motion, int direction) const
        {
            if(m_standMode){
                if(motion == MOTION_MON_SPECIAL){
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, direction);
                }
                else{
                    return ClientMonster::gfxID(motion, direction);
                }
            }
            else{
                if(motion == MOTION_MON_SPECIAL){
                    return ClientMonster::gfxID(MOTION_MON_APPEAR, direction); // lookID has been recirect to bigger dog
                }
                else{
                    return ClientMonster::gfxID(motion, direction);
                }
            }
        }
};
