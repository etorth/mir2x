/*
 * =====================================================================================
 *
 *       Filename: servertaodog.hpp
 *        Created: 04/10/2016 02:32:45
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
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerTaoDog final: public Monster
{
    private:
        bool m_standMode = false;

    private:
        int m_masterSC[2] = {0, 0};

    public:
        ServerTaoDog(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"神兽"), mapPtr, argX, argY, argDir, masterUID)
        {}

    public:
        void onActivate() override
        {
            CharObject::onActivate();
            fflassert(masterUID());

            m_actorPod->forward(masterUID(), {AM_CHECKMASTER}, [this](const ActorMsgPack &mpk)
            {
                switch(mpk.type()){
                    case AM_CHECKMASTEROK:
                        {
                            const auto amCMOK = mpk.conv<AMCheckMasterOK>();
                            m_masterSC[0] = amCMOK.sc[0];
                            m_masterSC[1] = amCMOK.sc[1];
                            return;
                        }
                    default:
                        {
                            goDie();
                            return;
                        }
                }
            });
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
                        .dog
                        {
                            .standModeReq = m_standMode,
                        },
                    },
                });
            }
        }

    protected:
        corof::eval_poller updateCoroFunc() override;

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
                    .dog
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
        DamageNode getAttackDamage(int) const override;
};
