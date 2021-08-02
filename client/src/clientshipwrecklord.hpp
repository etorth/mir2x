#pragma once
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "clientmonster.hpp"

class ClientShipwreckLord: public ClientMonster
{
    public:
        ClientShipwreckLord(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : ClientMonster(uid, proc, action)
        {
            fflassert(isMonster(u8"霸王教主"));
        }

    protected:
        bool onActionAttack(const ActionNode &) override;
};
