#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerTaoSummon: public Monster
{
    protected:
        int m_masterSC[2] = {0, 0};

    public:
        ServerTaoSummon(uint32_t argMonID, ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(argMonID, mapPtr, argX, argY, argDir, masterUID)
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
};
