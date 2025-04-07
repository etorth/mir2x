#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerCannibalPlant final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        ServerCannibalPlant(uint64_t argMapUID, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"食人花"), argMapUID, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .direction = DIR_BEGIN,
                .x = X(),
                .y = Y(),
                .extParam
                {
                    .cannibalPlant
                    {
                        .standMode = m_standMode,
                    },
                },
            };
        }

        void setStandMode(bool standMode)
        {
            if(standMode != m_standMode){
                m_standMode = standMode;
                dispatchAction(ActionTransf
                {
                    .direction = DIR_BEGIN,
                    .x = X(),
                    .y = Y(),
                    .extParam
                    {
                        .cannibalPlant
                        {
                            .standModeReq = standMode,
                        }
                    },
                });
            }
        }

    protected:
        bool struckDamage(uint64_t, const DamageNode &damage) override
        {
            if(!m_standMode){
                switch(damage.magicID){
                    case DBCOM_MAGICID(u8"火墙"):
                    case DBCOM_MAGICID(u8"地狱火"):
                    case DBCOM_MAGICID(u8"冰沙掌"):
                        {
                            setStandMode(true);
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                return true;
            }

            if(damage){
                updateHealth(-damage.damage);
                if(m_sdHealth.hp <= 0){
                    goDie();
                }
                return true;
            }
            return false;
        }
};
