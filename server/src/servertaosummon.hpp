#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerTaoSummon: public Monster
{
    protected:
        int m_masterSC[2] = {0, 0};

    public:
        ServerTaoSummon(uint32_t argMonID, uint64_t argMapUID, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(argMonID, argMapUID, argX, argY, argDir, masterUID)
        {}

    public:
        corof::awaitable<> onActivate() override
        {
            fflassert(masterUID());
            co_await CharObject::onActivate()();

            switch(const auto mpk = co_await m_actorPod->send(masterUID(), AM_CHECKMASTER); mpk.type()){
                case AM_CHECKMASTEROK:
                    {
                        const auto amCMOK = mpk.conv<AMCheckMasterOK>();
                        m_masterSC[0] = amCMOK.sc[0];
                        m_masterSC[1] = amCMOK.sc[1];
                        break;
                    }
                default:
                    {
                        goDie();
                        break;
                    }
            }
        }
};
