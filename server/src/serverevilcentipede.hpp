#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerEvilCentipede final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        ServerEvilCentipede(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"触龙神"), mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .x = X(),
                .y = Y(),

                .direction = DIR_BEGIN,
                .extParam
                {
                    .evilCentipede
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
                    .x = X(),
                    .y = Y(),

                    .direction = DIR_BEGIN,
                    .extParam
                    {
                        .evilCentipede
                        {
                            .standModeReq = standMode,
                        }
                    },
                });
            }
        }

    protected:
        bool struckDamage(const DamageNode &damage)
        {
            if(m_standMode){
                if(damage){
                    m_sdHealth.HP = std::max<int>(0, m_sdHealth.HP - damage.damage);
                    dispatchHealth();

                    if(m_sdHealth.HP <= 0){
                        goDie();
                    }
                    return true;
                }
            }
            return false;
        }
};
