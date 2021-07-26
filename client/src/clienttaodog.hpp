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
#include "clientstandmonster.hpp"

class ClientTaoDog: public ClientStandMonster
{
    public:
        ClientTaoDog(uint64_t, ProcessRun *, const ActionNode &);

    public:
        MonsterFrameGfxSeq getFrameGfxSeq(int motion, int) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_STAND  : return {.count =  4};
                    case MOTION_MON_WALK   : return {.count =  6};
                    case MOTION_MON_HITTED : return {.count =  2};
                    case MOTION_MON_DIE    : return {.count = 10};
                    case MOTION_MON_ATTACK0: return {.count =  6};
                    case MOTION_MON_APPEAR : return {.count = 10}; // from crawling to stand
                    default                : return {};
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND  : return {.gfxLookID = 0X59, .count =  4};
                    case MOTION_MON_WALK   : return {.gfxLookID = 0X59, .count =  6};
                    case MOTION_MON_HITTED : return {.gfxLookID = 0X59, .count =  2};
                    case MOTION_MON_DIE    : return {.gfxLookID = 0X59, .count = 10};
                    case MOTION_MON_SPECIAL:
                        {
                            // from non to crawling
                            // it's not used for standMode transf
                            return
                            {
                                .gfxLookID = 0X59,
                                .gfxMotionID = MOTION_MON_APPEAR,
                                .count = 10,
                            };
                        }
                    case MOTION_MON_APPEAR:
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

    protected:
        bool onActionStand (const ActionNode &) override;
        bool onActionSpawn (const ActionNode &) override;
        bool onActionTransf(const ActionNode &) override;
        bool onActionAttack(const ActionNode &) override;
};
