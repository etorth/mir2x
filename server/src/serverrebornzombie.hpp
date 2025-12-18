#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerRebornZombie final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        ServerRebornZombie(uint32_t monID, uint64_t argMapUID, int argX, int argY, int argDir)
            : Monster(monID, argMapUID, argX, argY, argDir, 0)
        {}

    protected:
        corof::awaitable<> runAICoro() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .direction = Direction(),
                .x = X(),
                .y = Y(),
                .extParam
                {
                    .rebornZombie
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
                    .direction = Direction(),
                    .x = X(),
                    .y = Y(),
                    .extParam
                    {
                        .rebornZombie
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
                if(m_sdHealth.dead()){
                    goDie();
                }
                return true;
            }
            return false;
        }
};
