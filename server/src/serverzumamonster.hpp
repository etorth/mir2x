#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerZumaMonster final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        ServerZumaMonster(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(monID, mapPtr, argX, argY, argDir, masterUID)
        {
            fflassert(isMonster(u8"祖玛雕像") || isMonster(u8"祖玛卫士"));
        }

    public:
        void setStandMode(bool standMode)
        {
            if(standMode != m_standMode){
                m_standMode = standMode;
                dispatchAction(ActionTransf
                {
                    .x = X(),
                    .y = Y(),
                    .direction = Direction(),
                    .extParam
                    {
                        .zumaMonster
                        {
                            .standModeReq = m_standMode,
                        },
                    },
                });
            }
        }

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .x = X(),
                .y = Y(),
                .direction = Direction(),
                .extParam
                {
                    .zumaMonster
                    {
                        .standMode = m_standMode,
                    },
                },
            };
        }

    protected:
        void onAMMasterHitted(const ActorMsgPack &) override
        {
            setStandMode(true);
        }

    protected:
        void onAMAttack(const ActorMsgPack &) override;

    protected:
        bool canMove() const override
        {
            return m_standMode && Monster::canMove();
        }
};
