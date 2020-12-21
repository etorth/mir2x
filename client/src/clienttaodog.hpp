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
        ClientTaoDog(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
            , m_standMode(action.ActionParam)
        {
            if(monsterName() != u8"神兽"){
                throw fflerror("bad monster type: %s", to_cstr(monsterName().data()));
            }
        }

    public:
        int motionFrameCount(int motion, int) const override
        {
            if(m_standMode){
                switch(motion){
                    case MOTION_MON_STAND   : return  4;
                    case MOTION_MON_WALK    : return  6;
                    case MOTION_MON_ATTACK0 : return  6;
                    case MOTION_MON_HITTED  : return  2;
                    case MOTION_MON_DIE     : return 10;
                    case MOTION_MON_APPEAR  : return 10; // from crawling to stand
                    default                 : return -1;
                }
            }
            else{
                switch(motion){
                    case MOTION_MON_STAND   : return  4;
                    case MOTION_MON_WALK    : return  6;
                    case MOTION_MON_HITTED  : return  2;
                    case MOTION_MON_DIE     : return 10;
                    case MOTION_MON_APPEAR  : return 10; // from non to crawling
                    default                 : return -1;
                }
            }
        }

    public:
        int lookID() const override
        {
            return m_standMode ? 0X5A : 0X59;
        }

    protected:
        bool onActionSpawn(const ActionNode &) override;
        bool onActionTransf(const ActionNode &) override;
        bool onActionAttack(const ActionNode &) override;

    public:
        bool visible() const override
        {
            return ClientCreature::active();
        }
};
