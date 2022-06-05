#pragma once
#include "dbcomid.hpp"
#include "clientmonster.hpp"

class ClientWedgeMoth: public ClientMonster
{
    public:
        ClientWedgeMoth(uint64_t, ProcessRun *, const ActionNode &);

    protected:
        bool onActionAttack(const ActionNode &) override;
};
