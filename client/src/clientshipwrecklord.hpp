#pragma once
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientShipwreckLord: public ClientMonster
{
    public:
        ClientShipwreckLord(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        bool onActionAttack(const ActionNode &) override;
};
