#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"
#include "fflerror.hpp"

class ServerZumaTaurus final: public Monster
{
    private:
        bool m_standMode = false;

    public:
        ServerZumaTaurus(uint64_t argMapUID, int argX, int argY, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"祖玛教主"), argMapUID, argX, argY, DIR_BEGIN, masterUID)
        {}

    public:
        void setStandMode(bool standMode)
        {
            fflassert(standMode);
            if(standMode != m_standMode){
                m_standMode = standMode;
                dispatchAction(ActionTransf
                {
                    .direction = Direction(),
                    .x = X(),
                    .y = Y(),
                    .extParam
                    {
                        .zumaTaurus
                        {
                            .standModeReq = m_standMode,
                        },
                    },
                });

                m_direction = DIR_DOWNLEFT;
                dispatchAction(makeActionStand());
            }
        }

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
                    .zumaTaurus
                    {
                        .standMode = m_standMode,
                    },
                },
            };
        }

    protected:
        corof::awaitable<bool> attackUID(uint64_t, int) override;

    protected:
        corof::awaitable<> onAMMasterHitted(const ActorMsgPack &) override
        {
            setStandMode(true);
            return {};
        }

    protected:
        corof::awaitable<> onAMAttack(const ActorMsgPack &) override;

    protected:
        bool canMove(bool checkMoveLock) const override
        {
            return m_standMode && Monster::canMove(checkMoveLock);
        }
};
